use concat_idents;

use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub enum SerumEventData {
    Level1(Level1Data),
    Level2Snapshot(Level2Data),
    Level2Update(Level2Data),
    #[default] Undefined,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SerumUnwrappedStreamEvent<Data> {
    pub stream: String,
    pub data: Data,
}

pub type SerumStreamEvent = SerumUnwrappedStreamEvent<SerumEventData>;

// #[macro_use]
// pub mod macros {
//     macro_rules! make_event_convert {
//         ($type:ident) => {
//             impl SerumUnwrappedStreamEvent<self::concat_idents!($type, Data)> {
//                 pub fn convert(self) -> SerumStreamEvent {
//                     SerumStreamEvent {
//                         stream: self.stream,
//                         data: SerumEventData::$type(self.data)
//                     }
//                 }
//             }
//         }
//     }
// }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Level1Data{
    #[serde(rename(deserialize = "type"))]
    pub type_msg: String,
    pub market: String,
    pub timestamp: String,
    pub slot: i64,
    pub version: i64,
    #[serde(rename(deserialize = "bestAsk"))]
    pub best_ask: Vec<String>,
    #[serde(rename(deserialize = "bestBid"))]
    pub best_bid: Vec<String>,
}
// make_event_convert!(Level1);

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Level2Data {
    #[serde(rename(deserialize = "type"))]
    pub type_msg: String,
    pub market: String,
    pub timestamp: String,
    pub slot: i64,
    pub version: i64,
    pub asks: Vec<Vec<String>>,
    pub bids: Vec<Vec<String>>,
}
// make_event_convert!(Level2);