#![feature(fn_traits)]
#![feature(stmt_expr_attributes)]
#![feature(associated_type_defaults)]
#![feature(never_type)]
#![feature(concat_idents)]
#![feature(new_uninit)]
#![feature(get_mut_unchecked)]
#![feature(trace_macros)]
#![feature(unboxed_closures)]

pub mod websocket;

use shared::all::websocket::WebSocketEvent;
use ws::Message;
use crate::websocket::core::SerumWebSocketClient;


pub struct SerumNativeConnector {
    pub websocket_client: SerumWebSocketClient
}

impl SerumNativeConnector {
    pub async fn new<Event: 'static, Handler: 'static + FnMut(WebSocketEvent<Event>) + Send + Sync, Parser: 'static + Fn(Message) -> Event + Send + Sync>(handler: Handler, parser: Parser) -> Self
    {
        let websocket_client = SerumWebSocketClient::new(handler, parser);
        Self {
            websocket_client
        }
    }
}
