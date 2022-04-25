pub use std::borrow::Borrow;
pub use std::hash::{Hash, Hasher};
pub use std::time;
pub use std::time::UNIX_EPOCH;
use chrono::{DateTime, TimeZone, NaiveDateTime, Utc};
pub use hmacsha::HmacSha;
pub use sha2::Sha256;
use crate::all::enums::{OrderSide, OrderStatus, OrderTypePrice, ExecType, TimeInForce, ExecTransType};

#[derive(Debug, Clone, PartialOrd)]
pub struct Order {
    cl_id: String,
    orig_qty: f64,
    rem_qty: f64,
    side: OrderSide,
    ord_type: OrderTypePrice,
    tif: TimeInForce,
    broker: String,
    sec_id: String,
    init_time: chrono::DateTime<chrono::Utc>,
}

impl Hash for Order {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.hash(state);
    }
}
impl PartialEq for Order
{
    fn eq(&self, other: &Self) -> bool {
        self.cl_id == other.cl_id
    }
}

#[derive(Debug)]
pub struct OrderSnapshot{
    order : Order,
    exch_id: String,
    status: OrderStatus,
    rem_qty: f64,
    last_qty: f64,
    last_price: f64,
    last_time: chrono::DateTime<chrono::Utc>,
}


#[derive(Debug)]
pub struct OrderUpdate{
    order : Order,
    orig_id: String,
    price: f64,
    qty: f64,
}

#[derive(Debug)]
pub struct OrderBook {
    asks: Vec<Level>,
    bids: Vec<Level>,
    instrument: Instrument
}

impl OrderBook {
    pub fn new(instrument: Instrument) -> Self {
        Self {
            asks: Vec::new(),
            bids: Vec::new(),
            instrument,
        }
    }

    pub fn update_ask(&mut self, level: Level) {
        // FIXME: potentially too slow function
        let res = self.asks.binary_search_by(|l| l.price.partial_cmp(&level.price).unwrap());
        match res {
            Ok(idx) => unsafe {
                *self.asks.get_unchecked_mut(idx) = level;
            },
            Err(idx) => self.asks.insert(idx, level),
        }
    }

    pub fn update_bid(&mut self, level: Level) {
        let res = self.bids.binary_search_by(|l| l.price.partial_cmp(&level.price).unwrap());
        match res {
            Ok(idx) => unsafe {
                *self.bids.get_unchecked_mut(idx) = level;
            },
            Err(idx) => self.bids.insert(idx, level),
        }
    }

    pub fn drop_zero_levels(&mut self) {
        self.asks.retain(|level| level.size != 0.0);
        self.bids.retain(|level| level.size != 0.0);
    }

    pub fn best_ask(&self) -> Option<&Level> {
        self.asks.get(0)
    }

    pub fn best_bid(&self) -> Option<&Level> {
        self.bids.get(0)
    }

    pub fn cbook_ask(&self, size: f64) -> Option<f64> {
        Self::cbook(&self.asks, size)
    }

    pub fn cbook_bid(&self, size: f64) -> Option<f64> {
        Self::cbook(&self.bids, size)
    }

    fn cbook(levels: &[Level], size: f64) -> Option<f64> {
        let mut res: f64 = 0.0;
        let mut valid_levels = Vec::with_capacity(30);
        for level in levels.iter() {
            res += level.size;
            valid_levels.push(level);
            if res >= size {
                break;
            }
        }

        if res < size {
            None
        } else {
            let sum = valid_levels
                .into_iter()
                .fold(0.0, |acc, level| acc + level.size) / res;
            Some(sum)
        }
    }

    pub fn get_instrument(&self) -> &Instrument {
        &self.instrument
    }

}

#[derive(Debug, Clone, PartialOrd)]
pub struct ExecutionReport {
    cl_id: String,
    exch_id : String,
    replace_cl_id: String,
    trade_id: String,
    broker : String,
    sec_id : String,
    text : String,
    ord_state: OrderStatus,
    exec_type: ExecType,
    trans_type: ExecTransType,
    side: OrderSide,
    ord_type: OrderTypePrice,
    last_time: chrono::DateTime<chrono::Utc>,
    limit: f64,
    stop: f64,
    last: f64,
    last_qty: f64,
    cum_qty: f64,
    leaves_qty: f64,
    total_qty: f64,
}

impl Hash for ExecutionReport
{
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.hash(state);
    }
}
impl PartialEq for ExecutionReport
{
    fn eq(&self, other: &Self) -> bool {
        self.trade_id == other.trade_id
    }
}

#[derive(Debug, Copy, Clone)]
pub struct Level {
    pub price: f64,
    pub size: f64
}

#[derive(Debug, Clone, PartialOrd)]
pub struct Instrument {
    symbol: String,
    base: String,
    currency : String,
    exchange: String,
    description: String,
    decimals: u8,
    value_precision: u8,
    lot_step: f64,
}

impl Hash for Instrument {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.hash(state);
    }
}
impl PartialEq for Instrument
{
    fn eq(&self, other: &Self) -> bool {
        self.base == other.base && self.currency == other.currency && self.exchange == other.exchange
    }
}

pub fn get_unix_time_millis() -> u128 {
    time::SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_millis()
}

pub fn make_signature(api_secret: &str, message: &str) -> String {
    hex::encode(HmacSha::new(api_secret.as_bytes(), message.as_bytes(), Sha256::default()).compute_digest())
}




//
/*
struct md_snapshot_t
{
    md_snapshot_t () :
    bid(-1.0),
    ask(-1.0),
    trade(-1.0),
    bid_size(0),
    ask_size(0),
    trade_size(0),
    time(0)
    {
    }

    double	bid;
    double  ask;
    double  trade;
    int		bid_size;
    int		ask_size;
    int     trade_size;
    ACE_Time_Value time;

    bool is_opened() const {return (bid != -1.0 && ask != -1.0 && bid != 0.0 && ask != 0.0);}
    bool is_bid() const{return bid != -1.0 && bid != 0.0;}
    bool is_ask() const{return ask != -1.0 && ask != 0.0;}
    double middle(int p) const
    {
    if(is_opened() == false) return 0.0;
    double v = (bid + ask)/2;
    v *= pow(10.0, p);
    v = floor(v+ 0.1);
    v /= pow(10.0, p);
    return v;
    }
};
*/
