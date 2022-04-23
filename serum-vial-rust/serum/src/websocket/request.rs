pub struct  WebSocketRequest {
    pub method : WebSocketMethod,
    pub channel : WebsocketChannel,
    pub pair: String
}

impl ToString for WebSocketRequest {
    fn to_string(&self) -> String {
        format!("{{\"op\":\"{}\",\"channel\":\"{}\",\"markets\":[\"{}\"]}}", self.method.to_string(), self.channel.to_string(), self.pair)
    }
}

pub enum WebSocketMethod {
    Subscribe,
    Unsubscribe,
}


impl std::fmt::Display for WebSocketMethod {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            WebSocketMethod::Subscribe => write!(f, "subscribe"),
            WebSocketMethod::Unsubscribe => write!(f, "unsubscribe"),
        }
    }
}

pub enum WebsocketChannel {
    Level1,
    Level2
}

impl std::fmt::Display for WebsocketChannel {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            WebsocketChannel::Level1 => write!(f, "level1"),
            WebsocketChannel::Level2 => write!(f, "level2"),
        }
    }
}

pub struct Instrument {
    first: String,
    second: String
}

impl Instrument {
    pub fn new(first: String, second: String) -> Self{
        Self{first, second}
    }

    pub fn get_market(&self) -> String {
        format!("{}/{}", self.first,self.second)
    }

    pub fn get_first(&self) -> String {
        format!("{}", self.first)
    }

    pub fn get_second(&self) -> String {
        format!("{}", self.second)
    }
}