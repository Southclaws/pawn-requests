#[macro_use]
extern crate samp_sdk;
extern crate futures;
extern crate reqwest;
extern crate tokio;
extern crate url;
#[macro_use]
extern crate enum_primitive;
extern crate num_traits;

mod method;
mod plugin;
mod pool;
mod request_client;

use plugin::Plugin;

new_plugin!(Plugin with process_tick);
