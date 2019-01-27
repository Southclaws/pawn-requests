use futures::{future, Future};
use std::{error::Error, sync::mpsc};
use string_error::static_err;
use tokio::runtime::Runtime;
use websocket::client::ClientBuilder;

pub struct WebsocketClient {
    runtime: Runtime,
    done_send: mpsc::Sender<String>,
    done_recv: mpsc::Receiver<String>,
}

impl WebsocketClient {
    pub fn new(endpoint: String) -> Result<WebsocketClient, Box<dyn Error>> {
        let url = url::Url::parse(&endpoint)?;
        if !url.scheme().starts_with("ws") {
            return Err(static_err("non-http scheme"));
        }
        let rt = Runtime::new()?;
        let (send, recv) = mpsc::channel();

        let client = ClientBuilder::from_url(&url)
            .async_connect(None)
            .and_then(|(duplex, _)| {
                let (sink, stream) = duplex.split();
                futures::select()
            })
            .map_err(|e| {
                //
                e
            });

        rt.spawn(client);

        Ok(WebsocketClient {
            runtime: rt,
            done_send: send,
            done_recv: recv,
        })
    }
}
