use futures::{future, Future, Stream};
use reqwest::{async::Client, header::HeaderMap, StatusCode};
use std::{error::Error, sync::mpsc};
use string_error::static_err;
use tokio::runtime::Runtime;

use method::Method;

pub struct RequestClient {
    runtime: Runtime,
    endpoint: String,
    headers: HeaderMap,
    client: Client,
    request_id: i32,
    done_send: mpsc::Sender<Response>,
    done_recv: mpsc::Receiver<Response>,
}

#[derive(Clone)]
pub struct Request {
    pub callback: String,
    pub path: String,
    pub method: Method,
    pub headers: HeaderMap,
    pub request_type: i32,
}

pub struct Response {
    pub request: Request,

    pub id: i32,
    pub body: String,
    pub status: StatusCode,
}

impl RequestClient {
    pub fn new(
        endpoint: String,
        headers: HeaderMap,
    ) -> Result<RequestClient, Box<std::error::Error>> {
        if !url::Url::parse(&endpoint)?.scheme().starts_with("http") {
            return Err(static_err("non-http scheme"));
        }
        let rt = Runtime::new()?;
        let (send, recv) = mpsc::channel();

        Ok(RequestClient {
            runtime: rt,
            endpoint: endpoint,
            headers: headers,
            client: Client::new(),
            request_id: 0,
            done_send: send,
            done_recv: recv,
        })
    }

    pub fn poll(&mut self) -> Result<Response, Box<dyn std::error::Error>> {
        Ok(self.done_recv.try_recv()?)
    }

    pub fn request(&mut self, request: Request) -> Result<i32, Box<Error>> {
        let id = self.request_id;
        let full = format!("{}/{}", self.endpoint, request.path);
        let request_copy = request.clone();
        let url = reqwest::Url::parse(&full)?;
        let sender = self.done_send.clone();

        self.request_id += 1;

        let req = self
            .client
            .request(request.method.into(), url)
            .headers(self.headers.clone())
            .headers(request.headers)
            .send()
            .map_err(|e| log!("{}", e))
            .and_then(move |mut response: reqwest::async::Response| {
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
                        log!("failed to read body: {}", e);
                        String::new()
                    }
                };

                sender
                    .send(Response {
                        request: request_copy,
                        id: id,
                        body: body,
                        status: response.status(),
                    })
                    .map_err(|e| log!("{}", e))
            })
            .map(|_| ());

        debug!("spawning request task for {} to {}", full, request.callback);
        self.runtime.spawn(req);

        Ok(id)
    }
}
