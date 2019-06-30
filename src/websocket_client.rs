use futures::sync;
use futures::Future;
use futures::Sink;
use futures::Stream;
use log::{debug, error};
use std::error::Error;
use string_error::static_err;
use tokio::runtime::Runtime;
use tokio::spawn;
use websocket::ClientBuilder;
use websocket::OwnedMessage;

pub struct WebsocketClient {
    pub callback: String,
    sender: sync::mpsc::Sender<OwnedMessage>,
    receiver: std::sync::mpsc::Receiver<OwnedMessage>,
    pub runtime: Runtime,
}

impl WebsocketClient {
    pub fn new(endpoint: String, callback: String) -> Result<WebsocketClient, Box<dyn Error>> {
        let url = url::Url::parse(&endpoint)?;
        if !url.scheme().starts_with("ws") {
            return Err(static_err("non-http scheme"));
        }

        let mut rt = Runtime::new()?;
        let (outgoing_send, outgoing_recv) = sync::mpsc::channel(4096);
        let (incoming_send, incoming_recv) = std::sync::mpsc::channel();

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
                            let _ = incoming_send.send(message);
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
            callback: callback,
            sender: outgoing_send,
            receiver: incoming_recv,
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

    pub fn poll(&mut self) -> Result<OwnedMessage, Box<dyn std::error::Error>> {
        Ok(self.receiver.try_recv()?)
    }
}
