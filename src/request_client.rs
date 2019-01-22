use futures::Future;
use reqwest::async::Client;
use reqwest::header::HeaderMap;
use reqwest::StatusCode;
use std::error::Error;
use std::sync::mpsc;
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
    pub fn new(endpoint: String, headers: HeaderMap) -> RequestClient {
        let rt = match Runtime::new() {
            Ok(v) => v,
            Err(e) => {
                panic!("Failed to create Tokio runtime: {}", e);
            }
        };

        let (send, recv) = mpsc::channel();

        RequestClient {
            runtime: rt,
            endpoint: endpoint,
            headers: headers,
            client: Client::new(),
            request_id: 0,
            done_send: send,
            done_recv: recv,
        }
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
