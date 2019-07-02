use log::{debug, error, info};
use reqwest::header::HeaderMap;
use samp::prelude::*;
use samp::SampPlugin;
use samp::{exec_public, native};
use serde_json;
use std::collections::HashMap;
use std::str::FromStr;
use string_error::new_err;

use crate::method::Method;
use crate::pool::{GarbageCollectedPool, Pool};
use crate::request_client::{Request, RequestClient, Response};
use crate::websocket_client::WebsocketClient;
use samp::amx::AmxIdent;

pub struct Plugin {
    pub request_clients: Pool<RequestClient>,
    pub request_client_amx: HashMap<i32, AmxIdent>,
    pub websocket_clients: Pool<WebsocketClient>,
    pub websocket_client_amx: HashMap<i32, AmxIdent>,
    pub json_nodes: GarbageCollectedPool<serde_json::Value>,
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

    /*fn on_amx_unload(&mut self, _Amx: &Amx) -> i32 {
        // let mut to_clear = Vec::new();
        // for (id, Amx_ptr) in self.request_client_amx.iter() {
        //     if Amx.Amx as usize == *Amx_ptr {
        //         to_clear.push(id);
        //     }
        // }
        // for id in to_clear.iter() {
        //     self.request_client_amx.remove(id);
        // }
        return Amx_ERR_NONE;
    }*/

    fn process_tick(&mut self) {
        for (id, rc) in self.request_clients.active.iter_mut() {
            let response: Response = match rc.poll() {
                Ok(v) => v,
                Err(_) => continue,
            };

            let raw = match self.request_client_amx.get(&id) {
                Some(v) => v,
                None => {
                    info!("orphan request client: lost handle to Amx");
                    continue;
                }
            };

            debug!(
                "Response {}: {}\n{}",
                response.id, response.request.callback, response.body
            );

            match execute_response_callback(*raw, response) {
                Ok(_) => (),
                Err(e) => error!("{}", e),
            };
        }

        for (id, wc) in self.websocket_clients.active.iter_mut() {
            let owned_message = match wc.poll() {
                Ok(v) => v,
                Err(_) => continue,
            };

            let response = match owned_message {
                websocket::OwnedMessage::Text(v) => v,
                _ => continue, // Todo: handle other cases
            };

            let raw = match self.websocket_client_amx.get(&id) {
                Some(v) => v,
                None => {
                    info!("orphan request client: lost handle to Amx");
                    continue;
                }
            };

            let callback = &wc.callback;

            debug!("WebSocket Response {}: {}\n{}", id, callback, response);
            if wc.is_json {
                let v: serde_json::Value = match serde_json::from_str(&response) {
                    Ok(v) => v,
                    Err(e) => {
                        error!("{}", e);
                        continue;
                    }
                };

                let node = self.json_nodes.alloc(v);
                match execute_json_websocket_callback(*raw, callback, id, &node) {
                    Ok(_) => (),
                    Err(e) => error!("{}", e),
                };
            } else {
                match execute_websocket_callback(*raw, callback, id, response) {
                    Ok(_) => (),
                    Err(e) => error!("{}", e),
                };
            }
        }
    }
}

impl Plugin {
    // Natives
    #[native(name = "RequestsClient")]
    pub fn requests_client(
        &mut self,
        amx: &Amx,
        endpoint: AmxString,
        _headers: i32, // TODO
    ) -> AmxResult<i32> {
        let header_map = HeaderMap::new();
        let endpoint = endpoint.to_string();

        let rqc = match RequestClient::new(endpoint.clone(), header_map) {
            Ok(v) => v,
            Err(e) => {
                error!("failed to create new client: {}", e);
                return Ok(-1);
            }
        };
        let id = self.request_clients.alloc(rqc);
        self.request_client_amx.insert(id, amx.ident());
        debug!(
            "created new request client {} with endpoint {}",
            id, endpoint
        );
        Ok(id)
    }

    #[native(raw, name = "RequestHeaders")]
    pub fn request_headers(&mut self, _: &Amx, mut params: samp::args::Args) -> AmxResult<i32> {
        let arg_count = params.count();
        let pairs = match arg_count == 0 || arg_count % 2 == 0 {
            true => arg_count / 2,
            false => {
                error!("invalid variadic argument pattern passed to JsonObject");
                return Ok(1);
            }
        };

        let mut headers = reqwest::header::HeaderMap::new();
        for _ in 0..pairs {
            let key = match params.next::<AmxString>() {
                None => {
                    error!("invalid type expected String");
                    return Ok(1);
                }
                Some(parameter) => parameter.to_string(),
            };
            let key = match reqwest::header::HeaderName::from_str(&key) {
                Ok(v) => v,
                Err(e) => {
                    error!("invalid header name {}: {}", key, e);
                    return Ok(1);
                }
            };

            let value = match params.next::<AmxString>() {
                None => {
                    error!("invalid type expected String");
                    return Ok(1);
                }
                Some(parameter) => parameter.to_string(),
            };

            let value = match reqwest::header::HeaderValue::from_str(&value) {
                Ok(v) => v,
                Err(e) => {
                    error!("invalid header value {}: {}", value, e);
                    return Ok(1);
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
        let headers = match self.headers.take(headers) {
            Some(v) => v,
            None => {
                error!("invalid headers identifier {} passed", headers);
                return Ok(1);
            }
        };
        let id = match self.do_request(
            request_client_id,
            path.to_string(),
            method,
            callback.to_string(),
            body.to_string(),
            headers,
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
        let body = match self.json_nodes.take(node) {
            Some(v) => v,
            None => {
                error!("invalid json node ID {}", node);
                return Ok(-1);
            }
        };
        let mut headers = match self.headers.take(headers) {
            Some(v) => v,
            None => {
                error!("invalid headers identifier {} passed", headers);
                return Ok(1);
            }
        };
        headers
            .insert("Content-Type", "application/json".parse().unwrap())
            .unwrap();
        let id = match self.do_request(
            request_client_id,
            path.to_string(),
            method,
            callback.to_string(),
            body,
            headers,
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
        headers: reqwest::header::HeaderMap,
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

        Ok(
            match client.request(Request {
                callback: callback,
                path: path,
                method: Method::from(method),
                body: body.to_string(),
                headers: headers,
                request_type: 0,
            }) {
                Ok(v) => v,
                Err(e) => {
                    error!("{}", e);
                    return Ok(1);
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

        let client = match WebsocketClient::new(endpoint.clone(), callback, false) {
            Ok(v) => v,
            Err(e) => {
                error!("failed to create new websocket client: {}", e);
                return Ok(-1);
            }
        };
        let id = self.websocket_clients.alloc(client);
        self.websocket_client_amx.insert(id, amx.ident());
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

        let client = match WebsocketClient::new(endpoint.clone(), callback, true) {
            Ok(v) => v,
            Err(e) => {
                error!("failed to create new websocket client: {}", e);
                return Ok(-1);
            }
        };
        let id = self.websocket_clients.alloc(client);
        self.websocket_client_amx.insert(id, amx.ident());
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

        let v: &serde_json::Value = match self.json_nodes.get(node) {
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

        *node = self.json_nodes.alloc(v);

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
        let v: &serde_json::Value = match self.json_nodes.get(node) {
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
        let v: &serde_json::Value = match self.json_nodes.get(node) {
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
        let pairs = match arg_count == 0 || arg_count % 2 == 0 {
            true => arg_count / 2,
            false => {
                error!("invalid variadic argument pattern passed to JsonObject");
                return Ok(1);
            }
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

            let node = match self.json_nodes.take(*node) {
                Some(v) => v,
                None => {
                    error!("invalid JSON node ID passed to JsonObject");
                    return Ok(2);
                }
            };

            v[key.to_string()] = node.clone();
        }

        Ok(self.json_nodes.alloc(v))
    }

    #[native(name = "JsonInt")]
    pub fn json_int(&mut self, _: &Amx, value: i32) -> AmxResult<i32> {
        Ok(self.json_nodes.alloc(serde_json::to_value(value).unwrap()))
    }

    #[native(name = "JsonBool")]
    pub fn json_bool(&mut self, _: &Amx, value: bool) -> AmxResult<i32> {
        Ok(self.json_nodes.alloc(serde_json::to_value(value).unwrap()))
    }

    #[native(name = "JsonFloat")]
    pub fn json_float(&mut self, _: &Amx, value: f32) -> AmxResult<i32> {
        Ok(self.json_nodes.alloc(serde_json::to_value(value).unwrap()))
    }

    #[native(name = "JsonString")]
    pub fn json_string(&mut self, _: &Amx, value: AmxString) -> AmxResult<i32> {
        Ok(self
            .json_nodes
            .alloc(serde_json::to_value(value.to_string()).unwrap()))
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

            let node = match self.json_nodes.take(*node) {
                Some(v) => v,
                None => {
                    error!("invalid JSON node ID passed to JsonArray");
                    return Ok(1);
                }
            };
            arr.push(node.clone());
        }
        Ok(self.json_nodes.alloc(serde_json::Value::Array(arr)))
    }

    #[native(name = "JsonAppend")]
    pub fn json_append(&mut self, _: &Amx, a: i32, b: i32) -> AmxResult<i32> {
        let a: serde_json::Value = match self.json_nodes.take(a) {
            Some(v) => v,
            None => return Ok(-1),
        };
        let b: serde_json::Value = match self.json_nodes.take(b) {
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
                return Ok(self.json_nodes.alloc(new));
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
                return Ok(self.json_nodes.alloc(new));
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
        let src: serde_json::Value = match self.json_nodes.take(value) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let dst: &mut serde_json::Value = match self.json_nodes.get(node) {
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
        let v: &mut serde_json::Value = match self.json_nodes.get(node) {
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
        let v: &mut serde_json::Value = match self.json_nodes.get(node) {
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
        let v: &mut serde_json::Value = match self.json_nodes.get(node) {
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
        let v: &mut serde_json::Value = match self.json_nodes.get(node) {
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
        let v: serde_json::Value = match self.json_nodes.get(node) {
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
        let v = self.json_nodes.alloc(v);
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
        let v: serde_json::Value = match self.json_nodes.get(node) {
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
        let v: serde_json::Value = match self.json_nodes.get(node) {
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
        let v: serde_json::Value = match self.json_nodes.get(node) {
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
        let v: serde_json::Value = match self.json_nodes.get(node) {
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
        let v: serde_json::Value = match self.json_nodes.get(node) {
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
        let v = self.json_nodes.alloc(v);
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
        let v: serde_json::Value = match self.json_nodes.get(node) {
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
        let v: serde_json::Value = match self.json_nodes.get(node) {
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
        let v = self.json_nodes.alloc(v);
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
        let v: serde_json::Value = match self.json_nodes.take(node) {
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
        let v: serde_json::Value = match self.json_nodes.take(node) {
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
        let v: serde_json::Value = match self.json_nodes.take(node) {
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
        let v: serde_json::Value = match self.json_nodes.take(node) {
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
        match self.json_nodes.set_gc(node, set) {
            Some(_) => Ok(0),
            None => Ok(1),
        }
    }

    #[native(name = "JsonCleanup")]
    pub fn json_cleanup(&mut self, _: &Amx, node: i32, auto: bool) -> AmxResult<i32> {
        match if auto {
            self.json_nodes.collect(node)
        } else {
            self.json_nodes.collect_force(node)
        } {
            Some(_) => Ok(0),
            None => Ok(1),
        }
    }
}

fn execute_response_callback(amx: AmxIdent, response: Response) -> AmxResult<()> {
    let amx = samp::amx::get(amx).unwrap();
    let length = response.body.len();
    let body = response.body;
    let status = response.status.as_u16() as i32;
    let id = response.id;
    let callback = response.request.callback;

    let _ = exec_public!(amx,&callback,id,status,&body => string ,length);

    Ok(())
}

fn execute_websocket_callback(
    amx: AmxIdent,
    callback: &str,
    client_id: &i32,
    response: String,
) -> AmxResult<()> {
    let length = response.len();
    let amx = samp::amx::get(amx).unwrap();
    let _ = exec_public!(amx,callback,client_id,&response => string ,length);
    Ok(())
}

fn execute_json_websocket_callback(
    amx: AmxIdent,
    callback: &str,
    client_id: &i32,
    node: &i32,
) -> AmxResult<()> {
    let amx = samp::amx::get(amx).unwrap();
    let _ = exec_public!(amx, callback, client_id, node);
    Ok(())
}
