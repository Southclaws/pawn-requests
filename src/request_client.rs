use futures::Future;
use reqwest::async::Client;
use reqwest::header::HeaderMap;

pub struct RequestClient {
    endpoint: String,
    headers: HeaderMap,
    client: Client,
}

impl RequestClient {
    pub fn new(endpoint: String, headers: HeaderMap) -> RequestClient {
        RequestClient {
            endpoint: endpoint,
            headers: headers,
            client: Client::new(),
        }
    }

    pub fn request(
        &self,
        path: String,
        method: reqwest::Method,
        headers: HeaderMap,
    ) -> Result<impl Future<Item = reqwest::async::Response, Error = reqwest::Error>, url::ParseError>
    {
        let full = format!("{}/{}", self.endpoint, path);

        match reqwest::Url::parse(&full) {
            Ok(url) => {
                return Ok(self
                    .client
                    .request(method, url)
                    .headers(self.headers.clone())
                    .headers(headers)
                    .send());
            }
            Err(e) => {
                println!("{}", e);
                return Err(e);
            }
        };
    }
}
