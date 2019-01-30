use futures::future;
use futures::sync;
use futures::Future;
use futures::Sink;
use futures::Stream;
use std::error::Error;
use string_error::static_err;
use tokio::runtime::Runtime;
use websocket::ClientBuilder;
use websocket::OwnedMessage;

pub struct WebsocketClient {
    pub callback: String,
    sender: sync::mpsc::Sender<OwnedMessage>,
    receiver: std::sync::mpsc::Receiver<String>,
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
            .and_then(move |(sink, stream)| {
                let sink: futures::stream::SplitSink<
                    tokio::codec::Framed<
                        std::boxed::Box<(dyn websocket::async::Stream + std::marker::Send)>,
                        websocket::async::MessageCodec<websocket::OwnedMessage>,
                    >,
                > = sink;
                let stream: futures::stream::SplitStream<
                    tokio::codec::Framed<
                        std::boxed::Box<(dyn websocket::async::Stream + std::marker::Send)>,
                        websocket::async::MessageCodec<websocket::OwnedMessage>,
                    >,
                > = stream;

                // TODO:
                // 1. Read the items from `stream` and send them to
                // `incoming_send` so they can be read via `self.poll`.
                // 2. Read the items from `outgoing_recv` that were added via
                // `self.send` and write them to `sink`.

                future::ok(())
            })
            .map_err(|e| {
                log!("{}", e);
                return ();
            });

        debug!("connecting to websocket");
        rt.spawn(f);

        Ok(WebsocketClient {
            callback: callback,
            sender: outgoing_send,
            receiver: incoming_recv,
        })
    }

    pub fn send<T: ToString>(&mut self, data: T) -> Result<(), Box<dyn Error>> {
        self.sender
            .clone()
            .send(OwnedMessage::Text(data.to_string()))
            .wait()?;

        Ok(())
    }

    pub fn poll(&mut self) -> Result<String, Box<dyn std::error::Error>> {
        Ok(self.receiver.try_recv()?)
    }
}
