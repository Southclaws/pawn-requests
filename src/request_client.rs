use futures::{future, Future, Stream};
use log::{debug, error};
use reqwest::{header::HeaderMap, r#async::Client};
use samp::{exec_public, AmxLockError, AsyncAmx};
use std::error::Error;
use std::sync::{Arc, Mutex};
use string_error::static_err;
use tokio::runtime::Runtime;

use crate::method::Method;
use crate::pool::GarbageCollectedPool;

pub struct RequestClient {
    pub amx: AsyncAmx,
    runtime: Runtime,
    endpoint: url::Url,
    headers: HeaderMap,
    client: Client,
    request_id: i32,
}

#[derive(Clone)]
pub struct Request {
    pub callback: String,
    pub path: String,
    pub method: Method,
    pub headers: Option<HeaderMap>,
    pub body: String,
}

impl RequestClient {
    pub fn new(
        amx: AsyncAmx,
        endpoint: String,
        headers: HeaderMap,
    ) -> Result<RequestClient, Box<dyn std::error::Error>> {
        let url = url::Url::parse(&endpoint)?;
        if !url.scheme().starts_with("http") {
            return Err(static_err("non-http scheme"));
        }
        let rt = Runtime::new()?;

        Ok(RequestClient {
            amx,
            runtime: rt,
            endpoint: url,
            headers,
            client: Client::new(),
            request_id: 0,
        })
    }

    pub fn request(
        &mut self,
        request: Request,
        json_nodes: Option<Arc<Mutex<GarbageCollectedPool<serde_json::Value>>>>,
    ) -> Result<i32, Box<dyn Error>> {
        let id = self.request_id;
        let mut full_url = self.endpoint.clone();
        let response_amx = self.amx.clone();
        let failure_amx = self.amx.clone();
        let callback = request.callback.clone();

        self.request_id += 1;
        full_url.set_path(&request.path);

        let mut req = self
            .client
            .request(request.method.into(), full_url.clone())
            .headers(self.headers.clone());
        if let Some(headers) = request.headers {
            req = req.headers(headers);
        }

        let request_future = req
            .body(request.body)
            .send()
            .map_err(move |e| {
                execute_request_failure_callback(failure_amx, id, e.status(), e.to_string());
            })
            .and_then(move |mut response: reqwest::r#async::Response| {
                debug!(
                    "received response for request {} status {}",
                    id,
                    response.status()
                );

                let body = match response
                    .body_mut()
                    .fold(Vec::new(), move |mut v, chunk| {
                        v.extend(&chunk[..]);
                        future::ok::<_, reqwest::Error>(v)
                    })
                    .and_then(move |chunks| future::ok(String::from_utf8(chunks).unwrap()))
                    .wait()
                {
                    Ok(v) => v,
                    Err(e) => {
                        error!("failed to read body: {}", e);
                        String::new()
                    }
                };

                execute_response_callback(
                    response_amx,
                    callback,
                    id,
                    i32::from(response.status().as_u16()),
                    body,
                    json_nodes,
                );

                future::ok(())
            });

        debug!(
            "spawning request task for {} to {}",
            full_url, request.callback
        );

        self.runtime.spawn(request_future);

        Ok(id)
    }
}

fn execute_request_failure_callback(
    amx: AsyncAmx,
    id: i32,
    error_code: Option<reqwest::StatusCode>,
    message: String,
) {
    let amx = match amx.lock() {
        Err(AmxLockError::AmxGone) => {
            error!("OnRequestFailure => AMX is gone");
            return;
        }
        Err(_) => {
            error!("OnRequestFailure => mutex is poisoned");
            return;
        }
        Ok(amx) => amx,
    };

    let status = match error_code {
        Some(code) => i32::from(code.as_u16()),
        None => -1,
    };

    let _ = exec_public!(amx,"OnRequestFailure",id,status,&message => string,message.len());
}

fn execute_response_callback(
    amx: AsyncAmx,
    callback: String,
    id: i32,
    status: i32,
    body: String,
    json_nodes: Option<Arc<Mutex<GarbageCollectedPool<serde_json::Value>>>>,
) {
    let amx = match amx.lock() {
        Err(AmxLockError::AmxGone) => {
            error!("{} => AMX is gone", callback);
            return;
        }
        Err(_) => {
            error!("{} => mutex is poisoned", callback);
            return;
        }
        Ok(amx) => amx,
    };

    if json_nodes.is_some() {
        let v: serde_json::Value = match serde_json::from_str(&body) {
            Ok(v) => v,
            Err(e) => {
                error!("{}", e);
                return;
            }
        };

        let nodes = json_nodes.unwrap();
        let mut nodes = nodes.lock().unwrap();
        let node = nodes.alloc(v);
        drop(nodes);

        let _ = exec_public!(amx, &callback, id, status, node);
    } else {
        let _ = exec_public!(amx,&callback,id,status,&body => string ,body.len());
    }
}
