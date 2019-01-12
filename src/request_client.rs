use reqwest::Client;

pub struct RequestClient {
    endpoint: String,
    headers: i32,
    client: Client,
}

impl RequestClient {
    pub fn new(endpoint: String, headers: i32) -> RequestClient {
        RequestClient {
            endpoint: endpoint,
            headers: headers,
            client: Client::new(),
        }
    }

    pub fn request(&self, method: String, path: String, headers: i32) {
        let u = match reqwest::Url::parse(&path) {
            Ok(url) => url,
            Err(e) => {
                println!("{}", e);
                return;
            }
        };
        self.client.request(reqwest::Method::GET, u);
    }
}
