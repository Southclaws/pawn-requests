use enum_primitive::enum_from_primitive;
use samp::cell::AmxCell;
use samp::amx::Amx;
use samp::error::{AmxResult,AmxError};

enum_from_primitive! {
#[derive(Debug, PartialEq, Clone,Copy)]
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

impl AmxCell<'_> for Method {
    fn from_raw(_amx: &Amx, cell: i32) -> AmxResult<Method> {
        match cell {
            0 => Ok(Method::HttpMethodGet),
            1 => Ok(Method::HttpMethodHead),
            2 => Ok(Method::HttpMethodPost),
            3 => Ok(Method::HttpMethodPut),
            4 => Ok(Method::HttpMethodDelete),
            5 => Ok(Method::HttpMethodConnect),
            6 => Ok(Method::HttpMethodOptions),
            7 => Ok(Method::HttpMethodTrace),            
            8 => Ok(Method::HttpMethodPatch),
            _ => Err(AmxError::Params)
        }

    }
    fn as_cell(&self) -> i32 {
        match self {
            Method::HttpMethodGet => 0,
            Method::HttpMethodHead => 1,
            Method::HttpMethodPost => 2,
            Method::HttpMethodPut => 3,
            Method::HttpMethodDelete => 4,
            Method::HttpMethodConnect => 5,
            Method::HttpMethodOptions => 6,
            Method::HttpMethodTrace => 7,
            Method::HttpMethodPatch => 8,
        }
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
