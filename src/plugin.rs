use log::{debug, error, info};
use reqwest::header::HeaderMap;
use samp::prelude::*;
use samp::SampPlugin;
use samp::{native, AmxAsyncExt};
use serde_json;
use std::str::FromStr;
use std::sync::{Arc, Mutex};
use string_error::new_err;

use crate::method::Method;
use crate::pool::{GarbageCollectedPool, Pool};
use crate::request_client::{Request, RequestClient};
use crate::websocket_client::WebsocketClient;

pub struct Plugin {
    pub request_clients: Pool<RequestClient>,
    pub websocket_clients: Pool<WebsocketClient>,
    pub json_nodes: Arc<Mutex<GarbageCollectedPool<serde_json::Value>>>,
    pub headers: GarbageCollectedPool<reqwest::header::HeaderMap>,
}

enum_from_primitive! {
#[derive(Debug, PartialEq, Clone)]
enum JsonNode {
    Number = 0,
    Boolean,
    String,
    Object,
    Array,
    Null,
}
}

impl SampPlugin for Plugin {
    fn on_load(&mut self) {
        info!("Loaded");
    }

    fn on_unload(self: Box<Self>) {
        info!("Unloaded");
    }
}

impl Plugin {
    // Natives
    #[native(name = "RequestsClient")]
    pub fn requests_client(
        &mut self,
        amx: &Amx,
        endpoint: AmxString,
        headers: i32,
    ) -> AmxResult<i32> {
        let header_map = if headers != -1 {
            match self.headers.take(headers) {
                Some(v) => v,
                None => {
                    error!("invalid headers identifier {} passed", headers);
                    return Ok(-1);
                }
            }
        } else {
            HeaderMap::new()
        };

        let endpoint = endpoint.to_string();

        let rqc = match RequestClient::new(amx.to_async(), endpoint.clone(), header_map) {
            Ok(v) => v,
            Err(e) => {
                error!("failed to create new client: {}", e);
                return Ok(-1);
            }
        };
        let id = self.request_clients.alloc(rqc);

        debug!(
            "created new request client {} with endpoint {}",
            id, endpoint
        );
        Ok(id)
    }

    #[native(raw, name = "RequestHeaders")]
    pub fn request_headers(&mut self, _: &Amx, mut params: samp::args::Args) -> AmxResult<i32> {
        let arg_count = params.count();
        let pairs = if arg_count == 0 || arg_count % 2 == 0 {
            arg_count / 2
        } else {
            error!("invalid variadic argument pattern passed to JsonObject");
            return Ok(-1);
        };

        let mut headers = reqwest::header::HeaderMap::new();
        for _ in 0..pairs {
            let key = match params.next::<AmxString>() {
                None => {
                    error!("invalid type expected String");
                    return Ok(-1);
                }
                Some(parameter) => parameter.to_string(),
            };
            let key = match reqwest::header::HeaderName::from_str(&key) {
                Ok(v) => v,
                Err(e) => {
                    error!("invalid header name {}: {}", key, e);
                    return Ok(-1);
                }
            };

            let value = match params.next::<AmxString>() {
                None => {
                    error!("invalid type expected String");
                    return Ok(-1);
                }
                Some(parameter) => parameter.to_string(),
            };

            let value = match reqwest::header::HeaderValue::from_str(&value) {
                Ok(v) => v,
                Err(e) => {
                    error!("invalid header value {}: {}", value, e);
                    return Ok(-1);
                }
            };

            headers.append(key, value);
        }

        Ok(self.headers.alloc(headers))
    }

    #[native(name = "Request")]
    pub fn request(
        &mut self,
        _: &Amx,
        request_client_id: i32,
        path: AmxString,
        method: Method,
        callback: AmxString,
        body: AmxString,
        headers: i32,
    ) -> AmxResult<i32> {
        let headers = if headers != -1 {
            match self.headers.take(headers) {
                Some(v) => Some(v),
                None => {
                    error!("invalid headers identifier {} passed", headers);
                    return Ok(-1);
                }
            }
        } else {
            None
        };

        let id = match self.do_request(
            request_client_id,
            path.to_string(),
            method,
            callback.to_string(),
            body.to_string(),
            headers,
            false,
        ) {
            Ok(v) => v,
            Err(e) => {
                error!("failed to execute request: {}", e);
                return Ok(-1);
            }
        };

        Ok(id)
    }

    #[native(name = "RequestJSON")]
    pub fn request_json(
        &mut self,
        _: &Amx,
        request_client_id: i32,
        path: AmxString,
        method: Method,
        callback: AmxString,
        node: i32,
        headers: i32,
    ) -> AmxResult<i32> {
        let nodes = self.json_nodes.clone();
        let mut nodes = nodes.lock().unwrap();

        let body = if node != -1 {
            match nodes.take(node) {
                Some(v) => v,
                None => {
                    error!("invalid json node ID {}", node);
                    return Ok(-1);
                }
            }
        } else {
            serde_json::Value::Null
        };

        let body = serde_json::to_string(&body).unwrap();

        let mut headers = if headers != -1 {
            match self.headers.take(headers) {
                Some(v) => v,
                None => {
                    error!("invalid headers identifier {} passed", headers);
                    return Ok(-1);
                }
            }
        } else {
            HeaderMap::new()
        };

        headers.insert("Content-Type", "application/json".parse().unwrap());

        let id = match self.do_request(
            request_client_id,
            path.to_string(),
            method,
            callback.to_string(),
            body,
            Some(headers),
            true,
        ) {
            Ok(v) => v,
            Err(e) => {
                error!("failed to execute request: {}", e);
                return Ok(-1);
            }
        };

        Ok(id)
    }

    pub fn do_request<T: ToString>(
        &mut self,
        request_client_id: i32,
        path: String,
        method: Method,
        callback: String,
        body: T,
        headers: Option<HeaderMap>,
        is_json: bool,
    ) -> Result<i32, Box<dyn std::error::Error>> {
        let client = match self.request_clients.get(request_client_id) {
            Some(v) => v,
            None => {
                return Err(new_err(&format!(
                    "attempted to request with invalid client {}",
                    request_client_id
                )));
            }
        };

        debug!(
            "executing new request {} with {:?} to {} with {:?} calling {}",
            request_client_id, headers, path, method, callback
        );

        let json_nodes = if is_json {
            Some(self.json_nodes.clone())
        } else {
            None
        };

        Ok(
            match client.request(
                Request {
                    callback,
                    path,
                    method,
                    body: body.to_string(),
                    headers,
                },
                json_nodes,
            ) {
                Ok(v) => v,
                Err(e) => {
                    error!("{}", e);
                    return Ok(-1);
                }
            },
        )
    }

    #[native(name = "WebSocketClient")]
    pub fn web_socket_client(
        &mut self,
        amx: &Amx,
        endpoint: AmxString,
        callback: AmxString,
    ) -> AmxResult<i32> {
        let endpoint = endpoint.to_string();
        let callback = callback.to_string();
        let client_id = self.websocket_clients.current + 1;

        let client =
            match WebsocketClient::new(amx.to_async(), endpoint.clone(), callback, client_id, None)
            {
                Ok(v) => v,
                Err(e) => {
                    error!("failed to create new websocket client: {}", e);
                    return Ok(-1);
                }
            };

        let id = self.websocket_clients.alloc(client);

        debug!(
            "created new web socket client {} with endpoint {}",
            id, endpoint
        );
        Ok(id)
    }

    #[native(name = "WebSocketSend")]
    pub fn web_socket_send(&mut self, _: &Amx, client: i32, data: AmxString) -> AmxResult<i32> {
        let client = match self.websocket_clients.get(client) {
            Some(v) => v,
            None => return Ok(-1),
        };
        match client.send(data.to_string()) {
            Ok(v) => v,
            Err(e) => {
                error!("failed to send websocket data: {}", e);
                return Ok(-1);
            }
        };
        Ok(0)
    }

    #[native(name = "JsonWebSocketClient")]
    pub fn json_web_socket_client(
        &mut self,
        amx: &Amx,
        endpoint: AmxString,
        callback: AmxString,
    ) -> AmxResult<i32> {
        let endpoint = endpoint.to_string();
        let callback = callback.to_string();
        let client_id = self.websocket_clients.current + 1;
        let client = match WebsocketClient::new(
            amx.to_async(),
            endpoint.clone(),
            callback,
            client_id,
            Some(self.json_nodes.clone()),
        ) {
            Ok(v) => v,
            Err(e) => {
                error!("failed to create new websocket client: {}", e);
                return Ok(-1);
            }
        };

        let id = self.websocket_clients.alloc(client);

        debug!(
            "created new json web socket client {} with endpoint {}",
            id, endpoint
        );
        Ok(id)
    }

    #[native(name = "JsonWebSocketSend")]
    pub fn json_web_socket_send(&mut self, _: &Amx, client: i32, node: i32) -> AmxResult<i32> {
        let client = match self.websocket_clients.get(client) {
            Some(v) => v,
            None => return Ok(-1),
        };

        let nodes = self.json_nodes.lock().unwrap();
        let mut nodes = nodes;

        let v: &serde_json::Value = match nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };

        let data = match serde_json::to_string(&v) {
            Ok(v) => v,
            Err(e) => {
                error!("{}", e);
                return Ok(1);
            }
        };

        match client.send(data) {
            Ok(v) => v,
            Err(e) => {
                error!("failed to send websocket data: {}", e);
                return Ok(-1);
            }
        };
        Ok(0)
    }

    #[native(name = "JsonParse")]
    pub fn json_parse(&mut self, _: &Amx, input: AmxString, mut node: Ref<i32>) -> AmxResult<i32> {
        let v: serde_json::Value = match serde_json::from_str(&input.to_string()) {
            Ok(v) => v,
            Err(e) => {
                error!("{}", e);
                return Ok(1);
            }
        };

        let mut nodes = self.json_nodes.lock().unwrap();
        *node = nodes.alloc(v);

        Ok(0)
    }

    #[native(name = "JsonStringify")]
    pub fn json_stringify(
        &mut self,
        _: &Amx,
        node: i32,
        output: UnsizedBuffer,
        length: usize,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: &serde_json::Value = match nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };

        let s = match serde_json::to_string(&v) {
            Ok(v) => v,
            Err(e) => {
                error!("{}", e);
                return Ok(1);
            }
        };

        let mut dest = output.into_sized_buffer(length);
        let _ = samp::cell::string::put_in_buffer(&mut dest, &s);

        Ok(0)
    }

    #[native(name = "JsonNodeType")]
    pub fn json_node_type(&mut self, _: &Amx, node: i32) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();
        let v: &serde_json::Value = match nodes.get(node) {
            Some(v) => v,
            None => &serde_json::Value::Null,
        };

        debug!("{:?}", v);

        let t: i32 = match v {
            serde_json::Value::Null => JsonNode::Null as i32,
            serde_json::Value::Bool(_) => JsonNode::Boolean as i32,
            serde_json::Value::Number(_) => JsonNode::Number as i32,
            serde_json::Value::String(_) => JsonNode::String as i32,
            serde_json::Value::Array(_) => JsonNode::Array as i32,
            serde_json::Value::Object(_) => JsonNode::Object as i32,
        };

        Ok(t)
    }

    #[native(raw, name = "JsonObject")]
    pub fn json_object(&mut self, _: &Amx, mut params: samp::args::Args) -> AmxResult<i32> {
        let arg_count = params.count();
        let pairs = if arg_count == 0 || arg_count % 2 == 0 {
            arg_count / 2
        } else {
            error!("invalid variadic argument pattern passed to JsonObject");
            return Ok(1);
        };

        let mut v = serde_json::Value::Object(serde_json::Map::new());
        for _ in 0..pairs {
            let key = match params.next::<AmxString>() {
                None => {
                    error!("invalid type expected String");
                    return Ok(2);
                }
                Some(parameter) => parameter,
            };

            let node = match params.next::<Ref<i32>>() {
                None => {
                    error!("invalid type expected int");
                    return Ok(2);
                }
                Some(parameter) => parameter,
            };

            let mut nodes = self.json_nodes.lock().unwrap();

            let node = match nodes.take(*node) {
                Some(v) => v,
                None => {
                    error!("invalid JSON node ID passed to JsonObject");
                    return Ok(2);
                }
            };

            v[key.to_string()] = node.clone();
        }

        let mut nodes = self.json_nodes.lock().unwrap();
        Ok(nodes.alloc(v))
    }

    #[native(name = "JsonInt")]
    pub fn json_int(&mut self, _: &Amx, value: i32) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();
        Ok(nodes.alloc(serde_json::to_value(value).unwrap()))
    }

    #[native(name = "JsonBool")]
    pub fn json_bool(&mut self, _: &Amx, value: bool) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();
        Ok(nodes.alloc(serde_json::to_value(value).unwrap()))
    }

    #[native(name = "JsonFloat")]
    pub fn json_float(&mut self, _: &Amx, value: f32) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();
        Ok(nodes.alloc(serde_json::to_value(value).unwrap()))
    }

    #[native(name = "JsonString")]
    pub fn json_string(&mut self, _: &Amx, value: AmxString) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();
        Ok(nodes.alloc(serde_json::to_value(value.to_string()).unwrap()))
    }

    #[native(raw, name = "JsonArray")]
    pub fn json_array(&mut self, _: &Amx, mut params: samp::args::Args) -> AmxResult<i32> {
        let args = params.count();

        let mut arr = Vec::<serde_json::Value>::new();
        for _ in 0..args {
            let node = match params.next::<Ref<i32>>() {
                None => {
                    error!("invalid type expected int");
                    return Ok(1);
                }
                Some(parameter) => parameter,
            };

            let mut nodes = self.json_nodes.lock().unwrap();
            let node = match nodes.take(*node) {
                Some(v) => v,
                None => {
                    error!("invalid JSON node ID passed to JsonArray");
                    return Ok(1);
                }
            };
            arr.push(node.clone());
        }

        let mut nodes = self.json_nodes.lock().unwrap();

        Ok(nodes.alloc(serde_json::Value::Array(arr)))
    }

    #[native(name = "JsonAppend")]
    pub fn json_append(&mut self, _: &Amx, a: i32, b: i32) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let a: serde_json::Value = match nodes.take(a) {
            Some(v) => v,
            None => return Ok(-1),
        };
        let b: serde_json::Value = match nodes.take(b) {
            Some(v) => v,
            None => return Ok(-1),
        };

        match (a.as_object(), b.as_object()) {
            (Some(oa), Some(ob)) => {
                let mut new = serde_json::Value::Object(serde_json::Map::new());
                for (k, v) in oa.iter() {
                    new.as_object_mut().unwrap().insert(k.clone(), v.clone());
                }
                for (k, v) in ob.iter() {
                    new.as_object_mut().unwrap().insert(k.clone(), v.clone());
                }
                return Ok(nodes.alloc(new));
            }
            _ => debug!("append: a and b are not both objects"),
        };

        match (a.as_array(), b.as_array()) {
            (Some(oa), Some(ob)) => {
                let mut new = serde_json::Value::Array(Vec::new());
                for v in oa.iter() {
                    new.as_array_mut().unwrap().push(v.clone());
                }
                for v in ob.iter() {
                    new.as_array_mut().unwrap().push(v.clone());
                }
                return Ok(nodes.alloc(new));
            }
            _ => debug!("append: a and b are not both arrays"),
        };

        debug!("failed to append: a and b are not both objects or arrays");

        Ok(2)
    }

    #[native(name = "JsonSetObject")]
    pub fn json_set_object(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        value: i32,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let src: serde_json::Value = match nodes.take(value) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let dst: &mut serde_json::Value = match nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };
        if !src.is_object() || !dst.is_object() {
            return Ok(1);
        }

        dst[key.to_string()] = src;
        Ok(0)
    }

    #[native(name = "JsonSetInt")]
    pub fn json_set_int(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        value: i32,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: &mut serde_json::Value = match nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };
        if !v.is_object() {
            return Ok(1);
        }

        v[key.to_string()] = serde_json::to_value(value).unwrap();
        Ok(0)
    }

    #[native(name = "JsonSetFloat")]
    pub fn json_set_float(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        value: f32,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: &mut serde_json::Value = match nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };
        if !v.is_object() {
            return Ok(1);
        }

        v[key.to_string()] = serde_json::to_value(value).unwrap();
        Ok(0)
    }

    #[native(name = "JsonSetBool")]
    pub fn json_set_bool(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        value: bool,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: &mut serde_json::Value = match nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };
        if !v.is_object() {
            return Ok(1);
        }

        v[key.to_string()] = serde_json::to_value(value).unwrap();
        Ok(0)
    }

    #[native(name = "JsonSetString")]
    pub fn json_set_string(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        value: AmxString,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: &mut serde_json::Value = match nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };
        if !v.is_object() {
            return Ok(1);
        }

        v[key.to_string()] = serde_json::to_value(value.to_string()).unwrap();
        Ok(0)
    }

    #[native(name = "JsonGetObject")]
    pub fn json_get_object(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        mut value: Ref<i32>,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(2),
        };
        let v = match v.get(&key.to_string()) {
            Some(v) => v.clone(),
            None => return Ok(3),
        };
        let v = nodes.alloc(v);
        *value = v;

        Ok(0)
    }

    #[native(name = "JsonGetInt")]
    pub fn json_get_int(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        mut value: Ref<i32>,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(1),
        };
        let v = match v.get(&key.to_string()) {
            Some(v) => v.clone(),
            None => return Ok(2),
        };
        let v = match v.as_i64() {
            Some(v) => v as i32,
            None => return Ok(3),
        };
        *value = v;

        Ok(0)
    }

    #[native(name = "JsonGetFloat")]
    pub fn json_get_float(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        mut value: Ref<f32>,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(1),
        };
        let v = match v.get(&key.to_string()) {
            Some(v) => v.clone(),
            None => return Ok(2),
        };
        let v = match v.as_f64() {
            Some(v) => v as f32,
            None => return Ok(3),
        };

        *value = v;

        Ok(0)
    }

    #[native(name = "JsonGetBool")]
    pub fn json_get_bool(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        mut value: Ref<bool>,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(1),
        };
        let v = match v.get(&key.to_string()) {
            Some(v) => v.clone(),
            None => return Ok(2),
        };
        let v = match v.as_bool() {
            Some(v) => v,
            None => return Ok(3),
        };
        *value = v;
        Ok(0)
    }

    #[native(name = "JsonGetString")]
    pub fn json_get_string(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        value: UnsizedBuffer,
        length: usize,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(1),
        };
        let v = match v.get(&key.to_string()) {
            Some(v) => v.clone(),
            None => return Ok(2),
        };
        let v = match v.as_str() {
            Some(v) => v,
            None => return Ok(3),
        };

        let mut dest = value.into_sized_buffer(length);
        let _ = samp::cell::string::put_in_buffer(&mut dest, &v);

        Ok(0)
    }

    #[native(name = "JsonGetArray")]
    pub fn json_get_array(
        &mut self,
        _: &Amx,
        node: i32,
        key: AmxString,
        mut value: Ref<i32>,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(1),
        };
        let v = match v.get(&key.to_string()) {
            Some(v) => v.clone(),
            None => return Ok(2),
        };
        match v.as_array() {
            Some(_) => (),
            None => return Ok(3),
        };
        let v = nodes.alloc(v);
        *value = v;
        Ok(0)
    }

    #[native(name = "JsonArrayLength")]
    pub fn json_array_length(
        &mut self,
        _: &Amx,
        node: i32,
        mut length: Ref<i32>,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_array() {
            Some(v) => v,
            None => return Ok(1),
        };
        *length = v.len() as i32;
        Ok(0)
    }

    #[native(name = "JsonArrayObject")]
    pub fn json_array_object(
        &mut self,
        _: &Amx,
        node: i32,
        index: i32,
        mut output: Ref<i32>,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_array() {
            Some(v) => v,
            None => return Ok(1),
        };
        let v = match v.get(index as usize) {
            Some(v) => v.clone(),
            None => return Ok(2),
        };
        let v = nodes.alloc(v);
        *output = v;
        Ok(0)
    }

    #[native(name = "JsonGetNodeInt")]
    pub fn json_get_node_int(
        &mut self,
        _: &Amx,
        node: i32,
        mut output: Ref<i32>,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.take(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_i64() {
            Some(v) => v as i32,
            None => return Ok(1),
        };
        *output = v;
        Ok(0)
    }

    #[native(name = "JsonGetNodeFloat")]
    pub fn json_get_node_float(
        &mut self,
        _: &Amx,
        node: i32,
        mut output: Ref<f32>,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.take(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_f64() {
            Some(v) => v as f32,
            None => return Ok(1),
        };
        *output = v;
        Ok(0)
    }

    #[native(name = "JsonGetNodeBool")]
    pub fn json_get_node_bool(
        &mut self,
        _: &Amx,
        node: i32,
        mut output: Ref<bool>,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.take(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_bool() {
            Some(v) => v,
            None => return Ok(1),
        };
        *output = v;
        Ok(0)
    }

    #[native(name = "JsonGetNodeString")]
    pub fn json_get_node_string(
        &mut self,
        _: &Amx,
        node: i32,
        output: UnsizedBuffer,
        length: usize,
    ) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        let v: serde_json::Value = match nodes.take(node) {
            Some(v) => v.clone(),
            None => {
                debug!("value under {} doesn't exist", node);
                return Ok(1);
            }
        };
        let v = match v.as_str() {
            Some(v) => v,
            None => {
                debug!("value is not a string {:?}", v);
                return Ok(1);
            }
        };
        let mut dest = output.into_sized_buffer(length);
        let _ = samp::cell::string::put_in_buffer(&mut dest, &v);

        Ok(0)
    }

    #[native(name = "JsonToggleGC")]
    pub fn json_toggle_gc(&mut self, _: &Amx, node: i32, set: bool) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        match nodes.set_gc(node, set) {
            Some(_) => Ok(0),
            None => Ok(1),
        }
    }

    #[native(name = "JsonCleanup")]
    pub fn json_cleanup(&mut self, _: &Amx, node: i32, auto: bool) -> AmxResult<i32> {
        let mut nodes = self.json_nodes.lock().unwrap();

        match if auto {
            nodes.collect(node)
        } else {
            nodes.collect_force(node)
        } {
            Some(_) => Ok(0),
            None => Ok(1),
        }
    }
}
