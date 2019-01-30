use futures::future;
use futures::sync;
use futures::Future;
use futures::Sink;
use futures::Stream;
use std::error::Error;
use string_error::static_err;
use tokio::runtime::Runtime;
use websocket::result::WebSocketError;
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
            .and_then(move |(duplex, _)| {
                let (sink, stream) = duplex.split();

                debug!("connected to websocket");

                stream
                    .filter_map(move |message| {
                        debug!("stream: {:?}", message);
                        let r = match message {
                            OwnedMessage::Close(e) => Some(OwnedMessage::Close(e)),
                            OwnedMessage::Ping(d) => Some(OwnedMessage::Pong(d)),
                            OwnedMessage::Text(v) => {
                                match incoming_send.clone().send(v) {
                                    Ok(_) => (),
                                    Err(e) => log!("failed to send to internal queues: {}", e),
                                }
                                None
                            }
                            _ => None,
                        };
                        return r;
                    })
                    .select(
                        outgoing_recv
                            .and_then(|d| {
                                debug!("outgoing_recv: {:?}", d);
                                future::ok(d)
                            })
                            .map_err(|_| {
                                debug!("sending");
                                return WebSocketError::NoDataAvailable;
                            }),
                    )
                    .forward(sink)
            })
            .map(|_| {})
            .map_err(|e: WebSocketError| {
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
