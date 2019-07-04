use futures::sync;
use futures::Future;
use futures::Sink;
use futures::Stream;
use log::{debug, error};
use samp::{exec_public, AmxLockError, AsyncAmx};
use std::error::Error;
use std::sync::{Arc, Mutex};
use string_error::static_err;
use tokio::runtime::Runtime;
use tokio::spawn;
use websocket::ClientBuilder;
use websocket::OwnedMessage;

use crate::pool::GarbageCollectedPool;

pub struct WebsocketClient {
    sender: sync::mpsc::Sender<OwnedMessage>,
    pub runtime: Runtime,
}

impl WebsocketClient {
    pub fn new(
        amx: AsyncAmx,
        endpoint: String,
        callback: String,
        client_id: i32,
        json_nodes: Option<Arc<Mutex<GarbageCollectedPool<serde_json::Value>>>>,
    ) -> Result<WebsocketClient, Box<dyn Error>> {
        let url = url::Url::parse(&endpoint)?;
        if !url.scheme().starts_with("ws") {
            return Err(static_err("non-http scheme"));
        }
        let json_nodes = json_nodes.clone();

        let mut rt = Runtime::new()?;
        let (outgoing_send, outgoing_recv) = sync::mpsc::channel(4096);

        //test connection before spawning
        ClientBuilder::from_url(&url).connect(None)?;

        let f = ClientBuilder::from_url(&url)
            .async_connect(None)
            .map(|(duplex, _)| duplex.split())
            .map_err(|_| ())
            .and_then(move |(sink, stream)| {
                spawn(
                    stream
                        .map_err(|err| {
                            error!("{}", err);
                            return ();
                        })
                        .for_each(move |message| {
                            match message {
                                OwnedMessage::Text(message) => {
                                    execute_websocket_callback(
                                        amx.clone(),
                                        &callback,
                                        client_id,
                                        &message,
                                        json_nodes.clone(),
                                    );
                                }
                                _ => (),
                            };

                            Ok(())
                        }),
                );

                outgoing_recv
                    .forward(sink.sink_map_err(|err| error!("{:?}", err)))
                    .map(|_| ())
            });

        debug!("connecting to websocket");
        rt.spawn(f);

        Ok(WebsocketClient {
            sender: outgoing_send,
            runtime: rt,
        })
    }

    pub fn send<T: ToString>(&mut self, data: T) -> Result<(), Box<dyn Error>> {
        self.sender
            .clone()
            .send(OwnedMessage::Text(data.to_string()))
            .wait()?;

        Ok(())
    }
}

fn execute_websocket_callback(
    amx: AsyncAmx,
    callback: &str,
    client_id: i32,
    message: &str,
    json_nodes: Option<Arc<Mutex<GarbageCollectedPool<serde_json::Value>>>>,
) {
    let amx = match amx.lock() {
        Err(AmxLockError::AmxGone) => {
            error!("{} => AMX is gone", callback);
            return;
        }
        Err(_) => {
            error!("{} => mutex is poisoned", callback);
            return;
        }
        Ok(amx) => amx,
    };
    if json_nodes.is_some() {
        let v: serde_json::Value = match serde_json::from_str(message) {
            Ok(v) => v,
            Err(e) => {
                error!("{}", e);
                return;
            }
        };

        let nodes = json_nodes.unwrap();
        let mut nodes = nodes.lock().unwrap();
        let node = nodes.alloc(v);
        drop(nodes);

        let _ = exec_public!(amx, callback, client_id, node);
    } else {
        let _ = exec_public!(amx,callback,client_id,message => string ,message.len());
    }
}
