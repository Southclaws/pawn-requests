use futures::{async, future, Future};
use std::{error::Error, sync};
use string_error::static_err;
use tokio::runtime::Runtime;
use websocket::client::ClientBuilder;

pub struct WebsocketClient {
    runtime: Runtime,
    done_send: sync::mpsc::Sender<String>,
    done_recv: sync::mpsc::Receiver<String>,
}

impl WebsocketClient {
    pub fn new(endpoint: String) -> Result<WebsocketClient, Box<dyn Error>> {
        let url = url::Url::parse(&endpoint)?;
        if !url.scheme().starts_with("ws") {
            return Err(static_err("non-http scheme"));
        }
        let rt = Runtime::new()?;
        let (send, recv) = sync::mpsc::channel();

        let (asend, arecv) = async::mpsc::channel(4096);

        let client = ClientBuilder::from_url(&url)
            .async_connect(None)
            .select(arecv.and_then(|a| {
                println!("{:?}", a);

                future::ok(())
            }))
            .and_then(|((a, c), b)| {
                println!("{:?}", a);

                future::ok(websocket::WebSocketError::NoDataAvailable)
            })
            .map_err(|(e, _)| {
                log!("{}", e);
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
