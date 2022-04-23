// #![feature(derive_default_enum)]
#![feature(stmt_expr_attributes)]
#![feature(associated_type_defaults)]
#![feature(never_type)]
#![feature(concat_idents)]
#![feature(trace_macros)]
#![feature(new_uninit)]

// #[macro_use]
// extern crate strum_macros;
// extern crate url;


use std::error::Error;
use std::thread::{sleep, yield_now};
use std::time::Duration;

use ws::Message;
use serum::SerumNativeConnector;
use serum::websocket::request::*;
pub use serum::websocket::response::{Level1Data, Level2Data, SerumEventData, SerumStreamEvent, SerumUnwrappedStreamEvent};
use shared::all::websocket::WebSocketEvent;


struct Container {
    asks: Vec<Vec<f64>>,
    bids: Vec<Vec<f64>>
}

impl Container{
    pub fn new() ->Self {
        Self{asks: Vec::new(), bids: Vec::new()}
    }

    pub fn get_asks(&self, mut num: usize) -> Vec<Vec<f64>> {
        if num > self.asks.len() {
            num = self.asks.len();
        }
        self.asks[..num].to_vec()
    }

    pub fn get_bids(&self, mut num: usize) -> Vec<Vec<f64>> {
        if num > self.bids.len() {
            num = self.bids.len();
        }
        self.bids[..num].to_vec()
    }

    pub fn update(&mut self, value: Vec<Vec<String>>, is_asks :bool) {
        let value :Vec<Vec<f64>> = value.into_iter().map(|x| vec![x[0].parse::<f64>().unwrap(), x[1].parse::<f64>().unwrap()] ).collect();
        
        let deleted: Vec<f64> = value.clone().into_iter().filter(|vec| vec[1] == 0.0).map(|x| x[0]).collect();
        let mut new: Vec<Vec<f64>> = value.clone().into_iter().filter(|vec| vec[1] != 0.0).collect();

        let old = match is_asks {
            true => self.asks.clone(),
            false => self.bids.clone()
        };

        let mut old : Vec<Vec<f64>> = old.clone().into_iter().filter(|x| !deleted.contains(&x[0])).collect();

        old.append(&mut new);

        old.sort_by(|a, b| a[0].partial_cmp(&b[0]).unwrap());


        match is_asks {
            true => self.asks = old,
            false => self.bids = {old.reverse(); old}
        }
    }
}


const NUM: usize = 5;


#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {

    let mut container : Container = Container::new();

    let connector = SerumNativeConnector::new(
        move |event: WebSocketEvent<SerumStreamEvent>| {
            // c.update_asks(40);
            println!(
                "{}",
                match event {
                    WebSocketEvent::Open => "Open".to_string(),
                    WebSocketEvent::Shutdown => "Shutdown".to_string(),
                    WebSocketEvent::Close(_, _) => "Close".to_string(),
                    WebSocketEvent::Error(_) => "Error".to_string(),
                    WebSocketEvent::Message(message) => match message.data {
                        SerumEventData::Level1(value) => format!("{} \nTop Bid {:?} \nTop Ask {:?}\n", value.market,
                        vec![value.best_bid[0].parse::<f64>().unwrap(), value.best_bid[1].parse::<f64>().unwrap()], 
                        vec![value.best_ask[0].parse::<f64>().unwrap(), value.best_ask[1].parse::<f64>().unwrap()]
                    ),
                        SerumEventData::Level2Snapshot(value) => {
                            container.update(value.asks.clone(), true);
                            container.update(value.bids.clone(), false); 
                            format!("{} \nBids {:?}\nAsks {:?}\n",  value.market, container.get_bids(NUM), container.get_asks(NUM))
                        },
                        SerumEventData::Level2Update(value) => {
                            container.update(value.asks.clone(), true);
                            container.update(value.bids.clone(), false); 
                            format!("{} \nBids {:?}\nAsks {:?}\n",  value.market, container.get_bids(NUM), container.get_asks(NUM))
                        },
                        _ => "UNDEFINED".to_string()
                    },
                    WebSocketEvent::Timeout(_) => "Timeout".to_string(),
                }
            );
        }, 
        |event| {
            let message = match event {
                Message::Text(t) => t,
                Message::Binary(b) => String::from_utf8(b).unwrap(),
            };
            if message.contains("subscribed") {
                println!("### {}", message);
                return SerumStreamEvent { stream: "".to_string(), data: SerumEventData::Undefined };
            }
            let event = message
                .split_once('"').unwrap().1
                .split_once('"').unwrap().1
                .split_once('"').unwrap().1
                .split_once('"').unwrap().0;
            
            match event {
                "quote" => SerumStreamEvent { stream: "".to_string(),  data: SerumEventData::Level1(serde_json::from_str::<Level1Data>(message.as_str()).unwrap())},
                "l2snapshot" => SerumStreamEvent { stream: "".to_string(),  data: SerumEventData::Level2Snapshot(serde_json::from_str::<Level2Data>(message.as_str()).unwrap())},
                "l2update" => SerumStreamEvent { stream: "".to_string(),  data: SerumEventData::Level2Update(serde_json::from_str::<Level2Data>(message.as_str()).unwrap())},
                _ => SerumStreamEvent { stream: "".to_string(),  data: SerumEventData::Undefined },
            }
    }).await;

    let eth_usdt = Instrument::new("ETH".to_string(), "USDC".to_string());
    let btc_usdt = Instrument::new("BTC".to_string(), "USDC".to_string());

    sleep(Duration::from_secs(1));


    // connector.websocket_client.subscribe(WebsocketChannel::Level1, &eth_usdt);
    // connector.websocket_client.subscribe(WebsocketChannel::Level2, &btc_usdt);

    loop {
        let mut input = String::new();
        std::io::stdin().read_line(&mut input).unwrap();
        // println!("{}", input);

        match &*input {
            "slv1\n" => connector.websocket_client.subscribe(WebsocketChannel::Level1, &eth_usdt),
            "ulv1\n" => connector.websocket_client.unsubscribe(WebsocketChannel::Level1, &eth_usdt),
            "slv2\n" => connector.websocket_client.subscribe(WebsocketChannel::Level2, &btc_usdt),
            "ulv2\n" => connector.websocket_client.unsubscribe(WebsocketChannel::Level2, &btc_usdt),
            _ => println!("{}", "unknown command")
        }
    }

}
