use std::sync::Arc;
use std::thread;
use crossbeam::sync::ShardedLock;
use ws::{CloseCode, Error, Handshake, Message, Request, Response, Sender};
use ws::util::{Token};

pub struct WebSocketClient {
    sender: Sender,
}

impl WebSocketClient {
    pub fn new<Event: 'static, Handler: 'static + FnMut(WebSocketEvent<Event>) + Send + Sync, Parser: 'static + Fn(Message) -> Event + Send + Sync>(
        url: String, handler: Handler, parser: Parser,
    ) -> Self {
        Self {
            sender: WebSocketWrapper::new(handler, parser).connect(url)
        }
    }
    pub fn send(&self, message: Message) {
        self.sender.send(message);
    }
}

pub struct WebSocketWrapper<Event, Handler: FnMut(WebSocketEvent<Event>), Parser: Fn(Message) -> Event> {
    handler: Arc<WebSocketHandler<Event, Handler, Parser>>,
}

impl<Event: 'static, Handler: 'static + FnMut(WebSocketEvent<Event>) + Send + Sync, Parser: 'static + Fn(Message) -> Event + Send + Sync> WebSocketWrapper<Event, Handler, Parser> {
    pub fn new(handler: Handler, parser: Parser) -> Self {
        Self {
            handler: Arc::new(WebSocketHandler::new(parser, handler))
        }
    }
    pub fn connect(self, url: String) -> Sender {
        let (s, r) = crossbeam::channel::unbounded();
        let (_s, _r) = crossbeam::channel::unbounded();
        _s.send(self.handler);
        thread::spawn(move || {
            let cp = _r.recv().unwrap();
            let mut websocket = ws::WebSocket::new(WebSocketFactory::new(cp)).unwrap();
            println!("{}", url);
            if let Err(err) = websocket.connect(url.parse().unwrap()) {
                println!("websocket connection error -> {}", err.details.to_string());
            }
            s.send(websocket.broadcaster());
            if let Err(err) = websocket.run() {
                println!("websocket run error -> {}", err.to_string());
            }
        });
        r.recv().unwrap()
    }
}

pub enum WebSocketEvent<Message> {
    Open,
    Shutdown,
    Close(CloseCode, String),
    Error(ws::Error),
    Message(Message),
    Timeout(Token),
}

pub type WebSocketRawEvent = WebSocketEvent<Message>;

impl WebSocketRawEvent {
    pub fn convert<Event, Parser: Fn(Message) -> Event>(self, parser: &Parser) -> WebSocketEvent<Event> {
        match self {
            WebSocketRawEvent::Open => WebSocketEvent::Open,
            WebSocketRawEvent::Shutdown => WebSocketEvent::Shutdown,
            WebSocketRawEvent::Close(code, reason) => WebSocketEvent::Close(code, reason),
            WebSocketRawEvent::Error(error) => WebSocketEvent::Error(error),
            WebSocketRawEvent::Message(message) => WebSocketEvent::Message(parser(message)),
            WebSocketRawEvent::Timeout(token) => WebSocketEvent::Timeout(token),
        }
    }
}

pub struct WebSocketHandler<Event, Handler: FnMut(WebSocketEvent<Event>), Parser: Fn(Message) -> Event> {
    handler: Arc<ShardedLock<Handler>>,
    parser: Arc<ShardedLock<Parser>>,
}

impl<Event, Handler: FnMut(WebSocketEvent<Event>), Parser: Fn(Message) -> Event> WebSocketHandler<Event, Handler, Parser> {
    pub fn new(parser: Parser, handler: Handler) -> Self {
        Self {
            handler: Arc::new(ShardedLock::new(handler)),
            parser: Arc::new(ShardedLock::new(parser)),
        }
    }
    pub fn handle(&mut self, event: WebSocketRawEvent) {
        (*self.handler.write().unwrap())(event.convert(&*self.parser.read().unwrap()));
    }
}

impl<Event, Handler: FnMut(WebSocketEvent<Event>), Parser: Fn(Message) -> Event> Clone for WebSocketHandler<Event, Handler, Parser> {
    fn clone(&self) -> Self {
        Self {
            handler: self.handler.clone(),
            parser: self.parser.clone(),
        }
    }
}

impl<Event, Handler: FnMut(WebSocketEvent<Event>), Parser: Fn(Message) -> Event> ws::Handler for WebSocketHandler<Event, Handler, Parser> {
    fn on_shutdown(&mut self) {
        println!("on_shutdown");
        self.handle(WebSocketRawEvent::Shutdown);
    }
    fn on_open(&mut self, shake: Handshake) -> ws::Result<()> {
        println!("on_open -> {}", shake.request.to_string());
        Ok(self.handle(WebSocketRawEvent::Open))
    }
    fn on_message(&mut self, message: Message) -> ws::Result<()> {
        // println!("on_message -> {}", message.to_string());
        Ok(self.handle(WebSocketRawEvent::Message(message)))
    }
    fn on_close(&mut self, code: CloseCode, reason: &str) {
        println!("on_close -> {}", reason.to_string());
        self.handle(WebSocketRawEvent::Close(code, String::from(reason)));
    }
    fn on_error(&mut self, error: Error) {
        println!("websocket error -> {}", error.details.to_string());
        self.handle(WebSocketRawEvent::Error(error));
    }
    fn on_timeout(&mut self, event: Token) -> ws::Result<()> {
        println!("on_timeout -> {}", event.0.to_string());
        Ok(self.handle(WebSocketRawEvent::Timeout(event)))
    }
    fn on_request(&mut self, req: &Request) -> ws::Result<Response> {
        println!("REQUEST");
        ws::Response::from_request(req)
    }
    fn on_response(&mut self, res: &Response) -> ws::Result<()> {
        println!("RESPONSE");
        Ok(())
    }
}

pub struct WebSocketFactory<Event, Handler: FnMut(WebSocketEvent<Event>), Parser: Fn(Message) -> Event> {
    handler: Arc<WebSocketHandler<Event, Handler, Parser>>,
}

impl<Event, Handler: FnMut(WebSocketEvent<Event>), Parser: Fn(Message) -> Event> WebSocketFactory<Event, Handler, Parser> {
    pub fn new(handler: Arc<WebSocketHandler<Event, Handler, Parser>>) -> Self {
        Self {
            handler
        }
    }
}

impl<Event, Handler: FnMut(WebSocketEvent<Event>), Parser: Fn(Message) -> Event> ws::Factory for WebSocketFactory<Event, Handler, Parser> {
    type Handler = WebSocketHandler<Event, Handler, Parser>;
    fn connection_made(&mut self, _: Sender) -> Self::Handler {
        println!("connection_made");
        self.handler.as_ref().clone()
    }
    fn client_connected(&mut self, ws: Sender) -> Self::Handler {
        println!("client_connected");
        self.connection_made(ws)
    }
}
