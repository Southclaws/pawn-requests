use futures::{Async, Stream};
use reqwest::header::HeaderMap;
use samp_sdk::amx::{AmxResult, AMX};
use samp_sdk::consts::*;
use samp_sdk::types::Cell;
use std::collections::HashMap;

use method::Method;
use pool::Pool;
use request_client::{Request, RequestClient, Response};

pub struct Plugin {
    request_clients: Pool<RequestClient>,
    request_client_amx: HashMap<i32, usize>,
}

define_native!(new_requests_client, endpoint: String, headers: i32);
define_native!(
    do_request,
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
define_native!(json_parse);
define_native!(json_stringify);
define_native!(json_node_type);
define_native!(json_object);
define_native!(json_int);
define_native!(json_bool);
define_native!(json_float);
define_native!(json_string);
define_native!(json_array);
define_native!(json_append);
define_native!(json_set_object);
define_native!(json_set_int);
define_native!(json_set_float);
define_native!(json_set_bool);
define_native!(json_set_string);
define_native!(json_get_object);
define_native!(json_get_int);
define_native!(json_get_float);
define_native!(json_get_bool);
define_native!(json_get_string);
define_native!(json_get_array);
define_native!(json_array_length);
define_native!(json_array_object);
define_native!(json_get_node_int);
define_native!(json_get_node_float);
define_native!(json_get_node_bool);
define_native!(json_get_node_string);
define_native!(json_toggle_gc);
define_native!(json_cleanup);

impl Plugin {
    pub fn load(&self) -> bool {
        return true;
    }

    pub fn unload(&self) {
        return;
    }

    pub fn amx_load(&self, amx: &AMX) -> Cell {
        let natives = natives! {
            "RequestsClient" => new_requests_client,
            "Request" => do_request,
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
            let r: Async<Option<Response>> = match rc.poll() {
                Ok(v) => v,
                Err(_) => continue,
            };

            if r.is_not_ready() {
                continue;
            }

            let raw = match self.request_client_amx.get(&id) {
                Some(v) => v,
                None => {
                    log!("orphan request client: lost handle to amx");
                    continue;
                }
            };
            let amx = cast_amx(raw);

            r.map(|o| {
                let response: Response = match o {
                    Some(v) => v,
                    None => return,
                };
                let public = match amx.find_public(&response.request.callback) {
                    Ok(v) => v,
                    Err(e) => {
                        log!("{}", e);
                        return;
                    }
                };

                println!("{}: {}", response.id, response.request.callback);

                match response_call_string(&amx, public, response) {
                    Ok(_) => (),
                    Err(e) => log!("{}", e),
                };
            });
        }
    }

    // Natives

    pub fn new_requests_client(
        &mut self,
        amx: &AMX,
        endpoint: String,
        _headers: i32, // TODO
    ) -> AmxResult<Cell> {
        let header_map = HeaderMap::new();
        let rqc = RequestClient::new(endpoint, header_map);
        let id = self.request_clients.alloc(rqc);
        self.request_client_amx.insert(id, amx.amx as usize);
        Ok(id)
    }

    pub fn do_request(
        &mut self,
        _: &AMX,
        request_client_id: Cell,
        path: String,
        method: Method,
        callback: String,
        _body: String,  // TODO
        _headers: Cell, // TODO
    ) -> AmxResult<Cell> {
        let client = match self.request_clients.get(request_client_id) {
            Some(v) => v,
            None => {
                return Ok(1);
            }
        };

        let header_map = HeaderMap::new();

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
    pub fn json_parse(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_stringify(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_node_type(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_object(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_int(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_bool(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_float(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_string(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_array(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_append(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_set_object(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_set_int(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_set_float(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_set_bool(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_set_string(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_get_object(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_get_int(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_get_float(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_get_bool(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_get_string(&mut self, _: &AMX) -> AmxResult<Cell> {
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
    pub fn json_get_node_int(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_get_node_float(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_get_node_bool(&mut self, _: &AMX) -> AmxResult<Cell> {
        Ok(0)
    }
    pub fn json_get_node_string(&mut self, _: &AMX) -> AmxResult<Cell> {
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
        {
            Plugin {
                request_clients: Pool::default(),
                request_client_amx: HashMap::new(),
            }
        }
    }
}

fn cast_amx(raw: &usize) -> AMX {
    AMX::new(*raw as *mut _)
}

fn response_call_string(amx: &AMX, public: Cell, response: Response) -> AmxResult<()> {
    amx.push(response.body.len())?;
    amx.push_string(&response.body, false)?;
    amx.push(response.status.as_u16())?;
    amx.push(response.id)?;
    amx.exec(public)?;
    Ok(())
}

// fn get_arg_ref<T: Clone>(amx: &AMX, parser: &mut args::Parser, out_ref: &mut T) -> i32 {
//     expand_args!(@amx, parser, tmp_ref: ref T);
//     *out_ref = tmp_ref.clone();
//     return 1;
// }

// fn get_arg_string(amx: &AMX, parser: &mut args::Parser, out_str: &mut String) -> i32 {
//     expand_args!(@amx, parser, tmp_str: String);
//     match samp_sdk::cp1251::decode_to(&tmp_str.into_bytes(), out_str) {
//         Ok(_) => {
//             return 1;
//         }
//         Err(e) => {
//             log!("{}", e);
//             return 0;
//         }
//     }
// }
