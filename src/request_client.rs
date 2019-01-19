use futures::{sync::mpsc, Future, Poll, Sink, Stream};
use reqwest::async::Client;
use reqwest::header::HeaderMap;
use reqwest::StatusCode;
use samp_sdk::amx::AMX;
use std::error::Error;
use tokio::runtime::Runtime;

use method::Method;

pub struct RequestClient {
    pub amx: AMX,
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

impl Stream for RequestClient {
    type Item = Response;
    type Error = ();

    fn poll(&mut self) -> Poll<Option<Response>, ()> {
        match self.done_recv.poll() {
            Ok(v) => Ok(v),
            Err(_) => Err(()),
        }
    }
}

impl RequestClient {
    pub fn new(amx: &AMX, endpoint: String, headers: HeaderMap) -> RequestClient {
        let rt = match Runtime::new() {
            Ok(v) => v,
            Err(e) => {
                panic!("Failed to create Tokio runtime: {}", e);
            }
        };

        let a = &*amx.to_owned();

        let (send, recv) = mpsc::channel(4096);

        RequestClient {
            amx: a,
            runtime: rt,
            endpoint: endpoint,
            headers: headers,
            client: Client::new(),
            request_id: 0,
            done_send: send,
            done_recv: recv,
        }
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
            .and_then(move |response| {
                sender
                    .send(Response {
                        request: request_copy,
                        id: id,
                        body: String::from(""),
                        status: response.status(),
                    })
                    .map_err(|e| log!("{}", e))
            })
            .map(|_| ());

        self.runtime.spawn(req);

        Ok(self.request_id)
    }
}
