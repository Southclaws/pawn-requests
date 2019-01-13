enum_from_primitive! {
#[derive(Debug, PartialEq)]
pub enum Method {
    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_PATCH,
}
}

impl Method {
    pub fn into(self) -> reqwest::Method {
        match self {
            Method::HTTP_METHOD_GET => reqwest::Method::GET,
            Method::HTTP_METHOD_HEAD => reqwest::Method::HEAD,
            Method::HTTP_METHOD_POST => reqwest::Method::POST,
            Method::HTTP_METHOD_PUT => reqwest::Method::PUT,
            Method::HTTP_METHOD_DELETE => reqwest::Method::DELETE,
            Method::HTTP_METHOD_CONNECT => reqwest::Method::CONNECT,
            Method::HTTP_METHOD_OPTIONS => reqwest::Method::OPTIONS,
            Method::HTTP_METHOD_TRACE => reqwest::Method::TRACE,
            Method::HTTP_METHOD_PATCH => reqwest::Method::PATCH,
        }
    }
}

impl From<Method> for reqwest::Method {
    fn from(w: Method) -> reqwest::Method {
        w.into()
    }
}
