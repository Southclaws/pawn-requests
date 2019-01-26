use reqwest::header::HeaderMap;
use samp_sdk::{
    amx::{AmxResult, AMX},
    args::Parser,
    consts::*,
    types::Cell,
};
use serde_json;
use std::collections::HashMap;

use method::Method;
use pool::Pool;
use request_client::{Request, RequestClient, Response};

pub struct Plugin {
    request_clients: Pool<RequestClient>,
    request_client_amx: HashMap<i32, usize>,
    json_nodes: Pool<serde_json::Value>,
}

define_native!(requests_client, endpoint: String, headers: i32);
define_native!(
    request,
    request_client_id: Cell,
    path: String,
    method: Method,
    callback: String,
    body: String,
    headers: Cell
);
define_native!(request_headers);
define_native!(request_json);
define_native!(web_socket_client);
define_native!(web_socket_send);
define_native!(json_web_socket_client);
define_native!(json_web_socket_send);
define_native!(json_parse, input: String, node: ref Cell);
define_native!(json_stringify, node: Cell, output: ref Cell, length: Cell);
define_native!(json_node_type, node: Cell);
define_native!(json_object as raw);
define_native!(json_int, value: Cell);
define_native!(json_bool, value: bool);
define_native!(json_float, value: f32);
define_native!(json_string, value: String);
define_native!(json_array as raw);
define_native!(json_append, a: Cell, b: Cell);
define_native!(json_set_object, node: Cell, key: String, value: Cell);
define_native!(json_set_int, node: Cell, key: String, value: Cell);
define_native!(json_set_float, node: Cell, key: String, value: f32);
define_native!(json_set_bool, node: Cell, key: String, value: bool);
define_native!(json_set_string, node: Cell, key: String, value: String);
define_native!(json_get_object, node: Cell, key: String, value: ref Cell);
define_native!(json_get_int, node: Cell, key: String, value: ref i32);
define_native!(json_get_float, node: Cell, key: String, value: ref f32);
define_native!(json_get_bool, node: Cell, key: String, value: ref bool);
define_native!(json_get_string, node: Cell, key: String, value: ref Cell, length: Cell);
define_native!(json_get_array);
define_native!(json_array_length);
define_native!(json_array_object);
define_native!(json_get_node_int, node: Cell, output: ref i32);
define_native!(json_get_node_float, node: Cell, output: ref f32);
define_native!(json_get_node_bool, node: Cell, output: ref bool);
define_native!(json_get_node_string, node: Cell, output: ref Cell, length: Cell);
define_native!(json_toggle_gc);
define_native!(json_cleanup);

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

impl Plugin {
    pub fn load(&self) -> bool {
        env_logger::init();
        return true;
    }

    pub fn unload(&self) {
        return;
    }

    pub fn amx_load(&self, amx: &AMX) -> Cell {
        let natives = natives! {
            "RequestsClient" => requests_client,
            "Request" => request,
            "RequestHeaders" => request_headers,
            "RequestJSON" => request_json,
            "WebSocketClient" => web_socket_client,
            "WebSocketSend" => web_socket_send,
            "JsonWebSocketClient" => json_web_socket_client,
            "JsonWebSocketSend" => json_web_socket_send,
            "JsonParse" => json_parse,
            "JsonStringify" => json_stringify,
            "JsonNodeType" => json_node_type,
            "JsonObject" => json_object,
            "JsonInt" => json_int,
            "JsonBool" => json_bool,
            "JsonFloat" => json_float,
            "JsonString" => json_string,
            "JsonArray" => json_array,
            "JsonAppend" => json_append,
            "JsonSetObject" => json_set_object,
            "JsonSetInt" => json_set_int,
            "JsonSetFloat" => json_set_float,
            "JsonSetBool" => json_set_bool,
            "JsonSetString" => json_set_string,
            "JsonGetObject" => json_get_object,
            "JsonGetInt" => json_get_int,
            "JsonGetFloat" => json_get_float,
            "JsonGetBool" => json_get_bool,
            "JsonGetString" => json_get_string,
            "JsonGetArray" => json_get_array,
            "JsonArrayLength" => json_array_length,
            "JsonArrayObject" => json_array_object,
            "JsonGetNodeInt" => json_get_node_int,
            "JsonGetNodeFloat" => json_get_node_float,
            "JsonGetNodeBool" => json_get_node_bool,
            "JsonGetNodeString" => json_get_node_string,
            "JsonToggleGC" => json_toggle_gc,
            "JsonCleanup" => json_cleanup
        };

        match amx.register(&natives) {
            Ok(_) => AMX_ERR_NONE,
            Err(err) => {
                log!("failed to register natives: {:?}", err);
                AMX_ERR_INIT
            }
        }
    }

    pub fn amx_unload(&mut self, _amx: &AMX) -> Cell {
        // let mut to_clear = Vec::new();
        // for (id, amx_ptr) in self.request_client_amx.iter() {
        //     if amx.amx as usize == *amx_ptr {
        //         to_clear.push(id);
        //     }
        // }
        // for id in to_clear.iter() {
        //     self.request_client_amx.remove(id);
        // }
        return AMX_ERR_NONE;
    }

    pub fn process_tick(&mut self) {
        for (id, rc) in self.request_clients.active.iter_mut() {
            let response: Response = match rc.poll() {
                Ok(v) => v,
                Err(_) => continue,
            };

            let raw = match self.request_client_amx.get(&id) {
                Some(v) => v,
                None => {
                    log!("orphan request client: lost handle to amx");
                    continue;
                }
            };
            let amx = cast_amx(raw);

            let public = match amx.find_public(&response.request.callback) {
                Ok(v) => v,
                Err(e) => {
                    log!("{}", e);
                    return;
                }
            };

            debug!(
                "Response {}: {}\n{}",
                response.id, response.request.callback, response.body
            );

            match response_call_string(&amx, public, response) {
                Ok(_) => (),
                Err(e) => log!("{}", e),
            };
        }
    }

    // Natives

    pub fn requests_client(
        &mut self,
        amx: &AMX,
        endpoint: String,
        _headers: i32, // TODO
    ) -> AmxResult<Cell> {
        let header_map = HeaderMap::new();
        let rqc = match RequestClient::new(endpoint.clone(), header_map) {
            Ok(v) => v,
            Err(e) => {
                log!("failed to create new client: {}", e);
                return Ok(-1);
            }
        };
        let id = self.request_clients.alloc(rqc);
        self.request_client_amx.insert(id, amx.amx as usize);
        debug!(
            "created new request client {} with endpoint {}",
            id, endpoint
        );
        Ok(id)
    }

    pub fn request(
        &mut self,
        _: &AMX,
        request_client_id: Cell,
        path: String,
        method: Method,
        callback: String,
        _body: String,  // TODO
        _headers: Cell, // TODO
    ) -> AmxResult<Cell> {
        log!("request called {} {:?} {}", path, method, callback);
        let client = match self.request_clients.get(request_client_id) {
            Some(v) => v,
            None => {
                debug!(
                    "attempted to request with invalid client {}",
                    request_client_id
                );
                return Ok(0);
            }
        };

        let header_map = HeaderMap::new();

        debug!(
            "executing new request with {} to {} with {:?} calling {}",
            request_client_id, path, method, callback
        );

        let id = match client.request(Request {
            callback: callback,
            path: path,
            method: Method::from(method),
            headers: header_map,
            request_type: 0,
        }) {
            Ok(v) => v,
            Err(e) => {
                log!("{}", e);
                return Ok(1);
            }
        };

        Ok(id)
    }

    // -
    // Not implemented yet:
    // -

    pub fn request_headers(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn request_json(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn web_socket_client(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn web_socket_send(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_web_socket_client(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_web_socket_send(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }

    pub fn json_parse(&mut self, _: &AMX, input: String, node: &mut Cell) -> AmxResult<Cell> {
        let v: serde_json::Value = match serde_json::from_str(&input) {
            Ok(v) => v,
            Err(e) => {
                log!("{}", e);
                return Ok(1);
            }
        };

        *node = self.json_nodes.alloc(v);

        Ok(0)
    }

    pub fn json_stringify(
        &mut self,
        _: &AMX,
        node: Cell,
        output: &mut Cell,
        length: Cell,
    ) -> AmxResult<Cell> {
        let v: &serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };

        let s = match serde_json::to_string(&v) {
            Ok(v) => v,
            Err(e) => {
                log!("{}", e);
                return Ok(1);
            }
        };
        let encoded: Vec<u8> = samp_sdk::cp1251::encode(&s)?;
        set_string!(encoded, output, length as usize);

        Ok(0)
    }

    pub fn json_node_type(&mut self, _: &AMX, node: Cell) -> AmxResult<Cell> {
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

    pub fn json_object(&mut self, amx: &AMX, params: *mut Cell) -> AmxResult<Cell> {
        let arg_count = args_count!(params);
        let pairs = match arg_count == 0 || arg_count % 2 == 0 {
            true => arg_count / 2,
            false => {
                log!("invalid variadic argument pattern passed to JsonObject");
                return Ok(1);
            }
        };
        let mut parser = Parser::new(params);

        let mut v = serde_json::Value::Object(serde_json::Map::new());
        for _ in 0..pairs {
            let mut key = String::new();
            get_arg_string(amx, &mut parser, &mut key);
            let mut node: Cell = 0;
            get_arg_ref(amx, &mut parser, &mut node);

            let node = match self.json_nodes.get(node) {
                Some(v) => v,
                None => {
                    log!("invalid JSON node ID passed to JsonObject");
                    return Ok(2);
                }
            };

            v[key] = node.clone();
        }

        Ok(self.json_nodes.alloc(v))
    }

    pub fn json_int(&mut self, _: &AMX, value: Cell) -> AmxResult<Cell> {
        Ok(self.json_nodes.alloc(serde_json::to_value(value).unwrap()))
    }
    pub fn json_bool(&mut self, _: &AMX, value: bool) -> AmxResult<Cell> {
        Ok(self.json_nodes.alloc(serde_json::to_value(value).unwrap()))
    }
    pub fn json_float(&mut self, _: &AMX, value: f32) -> AmxResult<Cell> {
        Ok(self.json_nodes.alloc(serde_json::to_value(value).unwrap()))
    }
    pub fn json_string(&mut self, _: &AMX, value: String) -> AmxResult<Cell> {
        Ok(self.json_nodes.alloc(serde_json::to_value(value).unwrap()))
    }

    pub fn json_array(&mut self, amx: &AMX, params: *mut Cell) -> AmxResult<Cell> {
        let args = args_count!(params);
        let mut parser = Parser::new(params);

        let mut arr = Vec::<serde_json::Value>::new();
        for _ in 0..args {
            let mut node: i32 = 0;
            get_arg_ref(amx, &mut parser, &mut node);
            let node = match self.json_nodes.get(node) {
                Some(v) => v,
                None => {
                    log!("invalid JSON node ID passed to JsonArray");
                    return Ok(1);
                }
            };
            arr.push(node.clone());
        }
        Ok(self.json_nodes.alloc(serde_json::Value::Array(arr)))
    }

    pub fn json_append(&mut self, _: &AMX, a: Cell, b: Cell) -> AmxResult<Cell> {
        let a: serde_json::Value = match self.json_nodes.get_const(a) {
            Some(v) => v.clone(),
            None => return Ok(-1),
        };
        let b: serde_json::Value = match self.json_nodes.get_const(b) {
            Some(v) => v.clone(),
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

    pub fn json_set_object(
        &mut self,
        _: &AMX,
        node: Cell,
        key: String,
        value: Cell,
    ) -> AmxResult<Cell> {
        let src: serde_json::Value = match self.json_nodes.get_const(value) {
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

        dst[key] = src;
        Ok(0)
    }

    pub fn json_set_int(
        &mut self,
        _: &AMX,
        node: Cell,
        key: String,
        value: Cell,
    ) -> AmxResult<Cell> {
        let v: &mut serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };
        if !v.is_object() {
            return Ok(1);
        }

        v[key] = serde_json::to_value(value).unwrap();
        Ok(0)
    }

    pub fn json_set_float(
        &mut self,
        _: &AMX,
        node: Cell,
        key: String,
        value: f32,
    ) -> AmxResult<Cell> {
        let v: &mut serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };
        if !v.is_object() {
            return Ok(1);
        }

        v[key] = serde_json::to_value(value).unwrap();
        Ok(0)
    }

    pub fn json_set_bool(
        &mut self,
        _: &AMX,
        node: Cell,
        key: String,
        value: bool,
    ) -> AmxResult<Cell> {
        let v: &mut serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };
        if !v.is_object() {
            return Ok(1);
        }

        v[key] = serde_json::to_value(value).unwrap();
        Ok(0)
    }

    pub fn json_set_string(
        &mut self,
        _: &AMX,
        node: Cell,
        key: String,
        value: String,
    ) -> AmxResult<Cell> {
        let v: &mut serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v,
            None => return Ok(1),
        };
        if !v.is_object() {
            return Ok(1);
        }

        v[key] = serde_json::to_value(value).unwrap();
        Ok(0)
    }

    pub fn json_get_object(
        &mut self,
        _: &AMX,
        node: Cell,
        key: String,
        value: &mut Cell,
    ) -> AmxResult<Cell> {
        let v: serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(2),
        };
        let v = match v.get(&key) {
            Some(v) => v.clone(),
            None => return Ok(3),
        };
        let v = self.json_nodes.alloc(v);
        *value = v;

        Ok(0)
    }

    pub fn json_get_int(
        &mut self,
        _: &AMX,
        node: Cell,
        key: String,
        value: &mut i32,
    ) -> AmxResult<Cell> {
        let v: serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(1),
        };
        let v = match v.get(&key) {
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

    pub fn json_get_float(
        &mut self,
        _: &AMX,
        node: Cell,
        key: String,
        value: &mut f32,
    ) -> AmxResult<Cell> {
        let v: serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(1),
        };
        let v = match v.get(&key) {
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

    pub fn json_get_bool(
        &mut self,
        _: &AMX,
        node: Cell,
        key: String,
        value: &mut bool,
    ) -> AmxResult<Cell> {
        let v: serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(1),
        };
        let v = match v.get(&key) {
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

    pub fn json_get_string(
        &mut self,
        _: &AMX,
        node: Cell,
        key: String,
        value: &mut Cell,
        length: Cell,
    ) -> AmxResult<Cell> {
        let v: serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_object() {
            Some(v) => v,
            None => return Ok(1),
        };
        let v = match v.get(&key) {
            Some(v) => v.clone(),
            None => return Ok(2),
        };
        let v = match v.as_str() {
            Some(v) => v,
            None => return Ok(3),
        };
        let encoded: Vec<u8> = samp_sdk::cp1251::encode(&v)?;
        set_string!(encoded, value, length as usize);
        Ok(0)
    }

    pub fn json_get_array(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_array_length(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_array_object(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }

    pub fn json_get_node_int(&mut self, _: &AMX, node: Cell, output: &mut i32) -> AmxResult<Cell> {
        let v: serde_json::Value = match self.json_nodes.get(node) {
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

    pub fn json_get_node_float(
        &mut self,
        _: &AMX,
        node: Cell,
        output: &mut f32,
    ) -> AmxResult<Cell> {
        let v: serde_json::Value = match self.json_nodes.get(node) {
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

    pub fn json_get_node_bool(
        &mut self,
        _: &AMX,
        node: Cell,
        output: &mut bool,
    ) -> AmxResult<Cell> {
        let v: serde_json::Value = match self.json_nodes.get(node) {
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

    pub fn json_get_node_string(
        &mut self,
        _: &AMX,
        node: Cell,
        output: &mut Cell,
        length: Cell,
    ) -> AmxResult<Cell> {
        let v: serde_json::Value = match self.json_nodes.get(node) {
            Some(v) => v.clone(),
            None => return Ok(1),
        };
        let v = match v.as_str() {
            Some(v) => v,
            None => return Ok(1),
        };
        let encoded: Vec<u8> = samp_sdk::cp1251::encode(&v)?;
        set_string!(encoded, output, length as usize);
        Ok(0)
    }

    pub fn json_toggle_gc(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_cleanup(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
}

impl Default for Plugin {
    fn default() -> Self {
        Plugin {
            request_clients: Pool::default(),
            request_client_amx: HashMap::new(),
            json_nodes: Pool::default(),
        }
    }
}

fn cast_amx(raw: &usize) -> AMX {
    AMX::new(*raw as *mut _)
}

fn to_pawn_string(input: String) -> Vec<i32> {
    let mut result = Vec::new();
    for char in input.as_bytes().iter() {
        result.push(*char as i32);
    }
    result.push(0); // EOS
    return result;
}

fn response_call_string(amx: &AMX, public: Cell, response: Response) -> AmxResult<()> {
    amx.push(response.body.len())?;
    let amx_addr = amx.push_array(to_pawn_string(response.body).as_slice())?;
    amx.push(response.status.as_u16() as i32)?;
    amx.push(response.id)?;
    amx.exec(public)?;
    amx.release(amx_addr)?;
    Ok(())
}

fn get_arg_ref<T: Clone>(amx: &AMX, parser: &mut Parser, out_ref: &mut T) -> i32 {
    expand_args!(@amx, parser, tmp_ref: ref T);
    *out_ref = tmp_ref.clone();
    return 1;
}

fn get_arg_string(amx: &AMX, parser: &mut Parser, out_str: &mut String) -> i32 {
    expand_args!(@amx, parser, tmp_str: String);
    match samp_sdk::cp1251::decode_to(&tmp_str.into_bytes(), out_str) {
        Ok(_) => {
            return 1;
        }
        Err(e) => {
            log!("{}", e);
            return 0;
        }
    }
}
