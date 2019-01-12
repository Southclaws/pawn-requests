#[macro_use]
extern crate samp_sdk;
extern crate reqwest;

mod plugin;
mod pool;
mod request_client;

use plugin::Plugin;

new_plugin!(Plugin with process_tick);
