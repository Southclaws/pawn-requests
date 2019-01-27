#[macro_use]
extern crate log;
extern crate env_logger;
#[macro_use]
extern crate samp_sdk;
#[macro_use]
extern crate enum_primitive;
extern crate futures;
extern crate num_traits;
extern crate reqwest;
extern crate serde_json;
extern crate string_error;
extern crate tokio;
extern crate url;
extern crate websocket;

mod method;
mod plugin;
mod pool;
mod request_client;
mod websocket_client;

use plugin::Plugin;

new_plugin!(Plugin with process_tick);
