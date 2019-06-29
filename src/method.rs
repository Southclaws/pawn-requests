enum_from_primitive! {
#[derive(Debug, PartialEq, Clone)]
pub enum Method {
    HttpMethodGet,
    HttpMethodHead,
    HttpMethodPost,
    HttpMethodPut,
    HttpMethodDelete,
    HttpMethodConnect,
    HttpMethodOptions,
    HttpMethodTrace,
    HttpMethodPatch,
}
}

impl Method {
    pub fn into(self) -> reqwest::Method {
        match self {
            Method::HttpMethodGet => reqwest::Method::GET,
            Method::HttpMethodHead => reqwest::Method::HEAD,
            Method::HttpMethodPost => reqwest::Method::POST,
            Method::HttpMethodPut => reqwest::Method::PUT,
            Method::HttpMethodDelete => reqwest::Method::DELETE,
            Method::HttpMethodConnect => reqwest::Method::CONNECT,
            Method::HttpMethodOptions => reqwest::Method::OPTIONS,
            Method::HttpMethodTrace => reqwest::Method::TRACE,
            Method::HttpMethodPatch => reqwest::Method::PATCH,
        }
    }
}

impl From<Method> for reqwest::Method {
    fn from(w: Method) -> reqwest::Method {
        w.into()
    }
}
