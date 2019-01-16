use futures::Future;
use reqwest::async::Client;
use reqwest::header::HeaderMap;

use method::Method;

pub struct RequestClient {
    endpoint: String,
    headers: HeaderMap,
    client: Client,
    request_id: i32,
}

pub struct Response {
    id: i32,
    callback: String,
    path: String,
    method: Method,
    requestType: i32,
    headers: i32,
    bodyString: String,
    bodyJson: String,
    status: i32,
    rawBody: String,
    isWebSocket: bool,
}

impl RequestClient {
    pub fn new(endpoint: String, headers: HeaderMap) -> RequestClient {
        RequestClient {
            endpoint: endpoint,
            headers: headers,
            client: Client::new(),
            request_id: 0,
        }
    }

    pub fn request(
        &mut self,
        path: String,
        method: reqwest::Method,
        headers: HeaderMap,
    ) -> Result<(i32, impl Future<Item = Response, Error = reqwest::Error>), url::ParseError> {
        let id = self.request_id;
        let full = format!("{}/{}", self.endpoint, path);

        self.request_id += 1;

        match reqwest::Url::parse(&full) {
            Ok(url) => {
                let r = self
                    .client
                    .request(method, url)
                    .headers(self.headers.clone())
                    .headers(headers)
                    .send()
                    .map(|response: reqwest::async::Response| {
                        // s
                        Response { id: id }
                    });

                Ok((id, r))
            }
            Err(e) => Err(e),
        }
    }
}
