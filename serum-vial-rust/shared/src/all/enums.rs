
//#[repr(u8)]
//https://www.onixs.biz/fix-dictionary/4.4/tagNum_40.html
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd)]
pub enum OrderType {
    Market,
    Limit,
    Stop,
    StopLimit,
}

//https://www.onixs.biz/fix-dictionary/4.4/tagNum_54.html
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd)]
pub enum OrderSide {
    Buy,
    Sell,
}

//https://www.onixs.biz/fix-dictionary/4.4/tagNum_39.html
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd)]
pub enum OrderStatus {
    New,
    PartFilled,
    Filled,
    Canceled,
    Replaced,
    PendingCancel,
    Rejected,
}

//https://www.onixs.biz/fix-dictionary/4.4/tagNum_150.html
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd)]
pub enum ExecType {
    New,
    PartFilled,
    Filled,
    Canceled,
    Replaced,
    PendingCancel,
    Rejected,
    PendingNew,
    Calculated,
    Expired,
    PendingReplace,
    Trade,
    OrderStatus,
}

//https://www.onixs.biz/fix-dictionary/4.4/tagNum_59.html
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd)]
pub enum TimeInForce {
    Day,
    GTC,
    OPG,
    IOC,
    FOK,
}

//https://www.onixs.biz/fix-dictionary/4.4/tagNum_20.html
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd)]
pub enum ExecTransType {
    New ,
    Cancel,
    Correct,
    Status,
}

//https://www.onixs.biz/fix-dictionary/4.4/tagNum_434.html
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd)]
pub enum OrdRejectResponse {
    CancelRequest,
    CancelReplaceRequest,
}

//https://www.onixs.biz/fix-dictionary/4.4/tagNum_380.html
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd)]
pub enum BusinessRejectReason {
    Other = 0,
    UnknownId = 1,
    UnknownSecurity = 2,
    UnsupportedMessageType = 3,
    ApplicationNotAvailable = 4,
    ConditionallyRequiredFieldMissing = 5
}

//https://www.onixs.biz/fix-dictionary/4.4/tagNum_167.html
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd)]
pub enum SecurityType {
    Forex,
    Index,
    Commodity,
    Treasury,
    Bullion,
    Stock,
    Bonds,
    Funds,
    Options,
    Futures,
    Cfd
}


