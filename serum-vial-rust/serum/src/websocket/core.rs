use ws::Message;
use shared::all::websocket::{WebSocketClient, WebSocketEvent};
use crate::websocket::request::*;

pub struct SerumWebSocketClient {
    inner: WebSocketClient
}

impl SerumWebSocketClient {
    pub fn new<Event: 'static, Handler: 'static + FnMut(WebSocketEvent<Event>) + Send + Sync, Parser: 'static + Fn(Message) -> Event + Send + Sync>(
        handler: Handler, parser: Parser
    ) -> Self {
         Self {
            inner: WebSocketClient::new(format!("wss://api.serum-vial.dev/v1/ws"), handler, parser)
        }
    }
    pub fn subscribe(&self, channel: WebsocketChannel, instr: &Instrument) {
        self.inner.send(Message::Text(WebSocketRequest{ method:WebSocketMethod::Subscribe, channel, pair: instr.get_market()}.to_string()))
    }
    pub fn unsubscribe(&self, channel: WebsocketChannel, instr: &Instrument) {
        self.inner.send(Message::Text(WebSocketRequest{ method:WebSocketMethod::Unsubscribe, channel, pair: instr.get_market()}.to_string()))
    }
}