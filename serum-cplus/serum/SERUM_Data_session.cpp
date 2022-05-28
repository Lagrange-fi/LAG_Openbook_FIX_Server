#include <chrono>
#include <string.h>
#include <functional>

#include "SERUM_Data_session.hpp"

bool _display_debug = true;

 class TestLogger: public ILogger
 {
 private:

     typedef std::string string;

 public:

     void Info(const char *content, ...) override;
     void Debug(const char *content, ...) override;
     void Error(const char *content, ...) override;
     void Critical(const char *content, ...) override;
     void Warn(const char *content, ...) override;
     void Trace(const char *content, ...) override;

     ~TestLogger() = default;
 };

void TestLogger::Info(const char *content, ...) {
    std::cout << "INFO | " << content << "\n";
}
void TestLogger::Debug(const char *content, ...) {
    std::cout << "INFO | " << content << "\n";
}
void TestLogger::Error(const char *content, ...) {
    std::cout << "INFO | " << content << "\n";
}
void TestLogger::Warn(const char *content, ...) {
    std::cout << "INFO | " << content << "\n";
}
void TestLogger::Critical(const char *content, ...) {
    std::cout << "CRITICAL | " << content << "\n";
}
void TestLogger::Trace(const char *content, ...) {
    std::cout << "TRACE | " << content << "\n";
}

class SerumSettings : public ISettings {

private:

    typedef std::string string;

public:

    string get(Property property) const override {
        switch (property) {
            case Property::ExchangeName:
                return "Serum";
            case Property::WebsocketEndpoint:
                return "wss://api.serum-vial.dev/v1/ws";
            default:
                return "";
        }
    }
};


SERUM_Data_session::SERUM_Data_session(const FIX8::F8MetaCntx& ctx,
                                       const FIX8::sender_comp_id& sci,
                                     FIX8::Persister *persist,
                                     FIX8::Logger *logger,
                                     FIX8::Logger *plogger):
        Session(ctx, sci, persist, logger, plogger),
        FIX8::SERUM_Data::FIX8_SERUM_Data_Router(),
        _logger(new TestLogger),
        _settings(new SerumSettings),
        _client( std::shared_ptr <SerumApp>(new SerumApp(_logger, this,_settings) ))
{
    std::cout << "SERUM_Data_session: SERUM_Data_session constructor " << std::endl;
}

/* SERUM_Data_session::~SERUM_Data_session()
{
    std::cout << "SERUM_Data_session:  destructor " << std::endl;
}*/


bool SERUM_Data_session::handle_application(const unsigned seqnum, const FIX8::Message *&msg)
{
    // _logger->LOG_INFO("LMAXTrade_session: handle_application\n");
    /*
    if(enforce(seqnum,msg))
    {
        std::cout << " enforce checking problem\n";
        return false;
    }
    if(!msg->process(*this) )
        detach(msg);
    return true;
    */
    std::cout << "SERUM_Data_session: handle_application\n";
    return enforce(seqnum, msg) || msg->process(*this);
}

FIX8::Message *SERUM_Data_session::generate_logon(const unsigned heartbeat_interval, const FIX8::f8String davi)
{
    std::cout << "SERUM_Data_session: generate_logon\n";
    FIX8::Message* logon = FIX8::Session::generate_logon(heartbeat_interval, davi);
    std::string username;
    /*if (_session_cfg->GetAttr("username", username))
    {
       *logon << new FIX8::LMAXTrade::Username(username);
    }
    std::string password;
    if (_session_cfg->GetAttr("password", password))
    {
        *logon << new FIX8::LMAXTrade::Password(password);
    }*/
    return logon;
}

bool SERUM_Data_session::handle_logon(const unsigned seqnum, const FIX8::Message *msg)
{
    std::cout << "SERUM_Data_session: handle_logon, " << std::endl;
    //_logger->LOG_INFO("SERUM_Data_session: handle_logon\n");
   // _is_connected=true;
   // _channel_listener->onEvent(_name, marketlib::channel_info::ci_logon,"");
   try {
       _client->start();
   }
   catch(std::exception& ex)
   {
       std::cout << "SERUM_Data_session, starting serumDEX " << ex.what() << std::endl;
   }
   return FIX8::Session::handle_logon(seqnum, msg);
}

bool SERUM_Data_session::handle_logout(const unsigned seqnum, const FIX8::Message *msg)
{
    std::cout << "SERUM_Data_session: handle_logout\n";
    //_logger->LOG_INFO("LMAXTrade_session: handle_logout\n");
    //_is_connected=false;
   // _channel_listener->onEvent(_name, marketlib::channel_info::ci_logout,"");
    try {
        _client->stop();
    }
    catch(std::exception& ex)
    {
        std::cout << "SERUM_Data_session, stopping serumDEX " << ex.what() << std::endl;
    }
    return FIX8::Session::handle_logon(seqnum, msg);
}

void SERUM_Data_session::modify_outbound(FIX8::Message *msg)
{
    std::cout << "SERUM_Data_session: modify_outbound, " << std::endl;
    if(_display_debug) {
        // std::ostringstream stream;
        // msg->print(stream);
        // _logger->LOG_DEBUG("---> %.200s\n",  stream.str().c_str());
    }
    return FIX8::Session::modify_outbound(msg);
}

bool SERUM_Data_session::process(const FIX8::f8String& from)
{
    std::cout << "SERUM_Data_session: process, " << from.c_str() << std::endl;
    if(_display_debug)
    {
        if(from.find("35=0") == -1)
            //_logger->LOG_INFO("<--- %s\n", from.c_str());
            std::cout << "<--- " << from.c_str() << std::endl;
    }
    return FIX8::Session::process(from);
}

bool SERUM_Data_session::operator() (const class FIX8::SERUM_Data::SecurityListRequest *msg) const
{
    /*
     320	SecurityReqID	Y	Y
     559	SecurityListRequestType	Y	Y
     Valid values:
     ‘4’  	All Pools
    */

    FIX8::SERUM_Data::SecurityReqID reqId;
    std::string reqIdStr;
    if(msg->get(reqId) && reqId.is_valid()){
        reqIdStr = reqId.get();
    }

    FIX8::SERUM_Data::SecurityListRequestType reqType;
    std::string reqTypeStr;
    if(msg->get(reqType)){
        reqTypeStr = reqTypeStr;
    }

    // test security list  response//
    std::list<marketlib::instrument_descr_t> pools{
        {.engine="SERUM", .sec_id="BTCUSDT", .symbol="BTCUSDT", .currency="USDT", .tick_precision=5},
        { .engine = "SERUM",.sec_id = "ETHUSDT",.symbol = "ETHUSDT",.currency = "USDT",.tick_precision = 5 },
    };
    auto* _sess = const_cast<SERUM_Data_session*>(this);
    _sess->securityList(reqIdStr,marketlib::security_request_result_t::srr_valid,pools);
    /////

    printf("SERUM_Data_session: Request pools: %s, %s\n", reqIdStr.c_str(), reqTypeStr.c_str());
    return false;
}

bool SERUM_Data_session::operator() (const class FIX8::SERUM_Data::MarketDataRequest *msg) const
{
    /*
    262 	MDReqID 	Y 	The ID of the Market Data Request. It must be new and unique unless 263=2 and refers to the previous market data request.
     */
    FIX8::SERUM_Data::MDReqID reqId;
    std::string reqIdStr;
    if(msg->get(reqId) && reqId.is_valid()){
        reqIdStr = reqId.get();
    }

    /*
      263 	SubscriptionRequestType 	Y 	Supported values:
        1	= Snapshot + Updates (e.g. “subscribe”)
        2	= Stop updates (e.g. “unsubscribe”)
     */
    marketlib::subscription_type subscr_type;
    FIX8::SERUM_Data::SubscriptionRequestType subscrReqId;
    if(msg->get(subscrReqId)){
        subscr_type = (marketlib::subscription_type)subscrReqId.get();
    }

    /*
     264 	MarketDepth 	Y 	The book depth to report.
        Supported values:

        0	= Full book, unlimited depth
        1	= Top of Book only
     */
    marketlib::market_depth_t mk_depth;
    FIX8::SERUM_Data::MarketDepth  depth_model;
    if(msg->get(depth_model)){
        mk_depth = (marketlib::market_depth_t)depth_model.get();
    }

    /*
     265 	MDUpdateType 	Y 	The format of market data updates.

        Supported values:

        0	= Full refresh (e.g. snapshot)
        1	= Incremental refresh (does not support at the moment)
                    Begin Repeating Group
    */
    marketlib::subscription_update_type_t update_type;
    FIX8::SERUM_Data::MDUpdateType   fix_update_type;
    if(msg->get(fix_update_type)){
        update_type = (marketlib::subscription_update_type_t)fix_update_type.get();
    }

    /*
   146 	NoRelatedSym 	Y 	The number of symbols in the request. The tag must be always set to 1.
 	 	 	Begin Repeating Group
        55 	Symbol 	Y 	The pool symbol expressed as CCY1/CCY2. For example, “DOT/USDT”.
            End Repeating Group
    */
    const FIX8::GroupBase *noRelatedSymGroup(msg->find_group<FIX8::SERUM_Data::MarketDataRequest::NoRelatedSym>());
    assert(noRelatedSymGroup!=nullptr);
    auto element = noRelatedSymGroup->get_element(0);
    FIX8::SERUM_Data::Symbol symbol;
    element->get(symbol);

    /*
     269 	MDEntryType 	Y 	Supported values:

        0	= Bid
        1	= Ask
        2	= Trade
     */

    marketlib::market_data_request_t request{
        "SERUM",
        symbol.get(),
        mk_depth,
        update_type
    };

    IBrokerClient::SubscriptionModel model = mk_depth ==
                                             marketlib::market_depth_t::top?
                                             IBrokerClient::SubscriptionModel::TopBook:
                                             IBrokerClient::SubscriptionModel::FullBook;
    BrokerModels::Instrument pool {.exchange = "SERUM", .symbol=symbol.get()};
    if(subscr_type==marketlib::subscription_type::shapshot_update) {
        printf("SERUM_Data_session: MD subscribe to %s:%s, depth(%d), update type(%d)\n",
               request.engine.c_str(),
               request.symbol.c_str(),
               request.depth,
               request.update_type);
        try{
            _client->subscribe(pool, model);
        }
        catch(std::exception& ex)
        {
            std::cout << "SERUM_Data_session, DEX Subscribe to " << symbol <<std::endl;
        }
    }
    if(subscr_type==marketlib::subscription_type::snapshot_update_disable) {
        printf("SERUM_Data_session: MD unsubscribe to %s:%s, depth(%d), update type(%d)\n",
               request.engine.c_str(),
               request.symbol.c_str(),
               request.depth,
               request.update_type);
        try
        {
            _client->unsubscribe(pool, model);
        }
        catch(std::exception& ex)
        {
            std::cout << "SERUM_Data_session, DEX Unsubscribe to " << symbol <<std::endl;
        }
    }

    return false;
}

void SERUM_Data_session::securityList(const std::string &reqId, marketlib::security_request_result_t result,
                                      const std::list<marketlib::instrument_descr_t>& pools)
{
    /*
    320	SecurityReqID	Y	Y
    322	SecurityResponseID	Y	Y	Identifier for the Security List (y) message
    560	SecurityRequestResult	Y	Y	The results returned to a Security Request (v) message
    Valid values:
    '0' 	Valid request
    '1' 	Invalid or unsupported request
    '2' 	No instruments found that match selection criteria
    '3' 	Not authorized to retrieve instrument data
    '4' 	Instrument data temporarily unavailable
    '5' 	Request for instrument data not supported
    component block  <Instrument>	N N
     */
    auto *mdr(new FIX8::SERUM_Data::SecurityList);

    *mdr    << new FIX8::SERUM_Data::SecurityReqID(reqId)
            << new FIX8::SERUM_Data::SecurityResponseID("resp" + reqId)
            << new FIX8::SERUM_Data::SecurityRequestResult(result)
            << new FIX8::SERUM_Data::NoRelatedSym(pools.size()) ;

    FIX8::GroupBase *noin(mdr->find_group<FIX8::SERUM_Data::SecurityList::NoRelatedSym >());
    for(const auto& pool_info : pools)
    {
        FIX8::MessageBase *noin_sym(noin->create_group());
        *noin_sym << new FIX8::SERUM_Data::SecurityExchange(pool_info.engine)
                  << new FIX8::SERUM_Data::Symbol (pool_info.symbol);
        *noin << noin_sym;
    }
    *mdr << noin;

    std::ostringstream str;
    mdr->print(str);
    std::cout << "--> " << str.str() << std::endl;
    FIX8::Session::send(mdr);
}

void SERUM_Data_session::marketReject(const std::string& reqId, marketlib::ord_rej_reason reason)
{
    /*
     Header 	Y 	Standard header, with 35=Y.
        MDReqID 	Y 	The ID of the Market Data Request.
        MDRecRejReason 	Y 	The reason for the rejection of the Market Data request.
        Text 	Y 	Message.
     */

    auto *mdr(new FIX8::SERUM_Data::MarketDataRequest);

    *mdr    << new FIX8::SERUM_Data::MDReqID(reqId)
            << new FIX8::SERUM_Data::MDReqRejReason (reason)
            ;

    FIX8::Session::send(mdr);
}


void SERUM_Data_session::fullSnapshot(const std::string& reqId, const marketlib::instrument_descr_t& sec_id,
                                 const BrokerModels::MarketBook & book)
{
    /*
        262 	MDReqID 	Y 	The ID of the Market Data Request that triggered sending this snapshot.
        55 	    Symbol 	Y 	The pool expressed as CCY1/CCY2. For example, “DOT/USDT”.
        268 	NoMDEntries 	Y 	The number of market data entries in this message.
                    Begin Repeating Group
        269 	MDEntryType 	Y 	Supported values:

        0	= Bid
        1	= Ask
        2	= Trade


        270 	MDEntryPx 	Y 	The market data entry price.
        271 	MDEntrySize 	Y 	The market data entry volume.
        64 	    SettlDate 	Y 	The Settlement date expressed as YYYYMMDD. For example, “19701231”
        278 	MDEntryID 	Y 	The unique ID for this market data.
                    End Repeating Group
     */
     auto *mdr(new FIX8::SERUM_Data::MarketDataSnapshotFullRefresh);
     *mdr   << new FIX8::SERUM_Data::Symbol (sec_id.symbol)
            << new FIX8::SERUM_Data::SecurityExchange (sec_id.engine)
            << new FIX8::SERUM_Data::NoMDEntries(1)
            ;

    {
        FIX8::GroupBase *nomd(mdr->find_group< FIX8::SERUM_Data::MarketDataSnapshotFullRefresh::NoMDEntries >());

        time_t update_time = std::chrono::system_clock::to_time_t(book.time);
        char* update_time_str = ctime(&update_time);
        FIX8::MessageBase *nomd_bid(nomd->create_group());
        *nomd_bid << new FIX8::SERUM_Data::MDEntryType(FIX8::SERUM_Data::MDEntryType_BID); // bids
        *nomd_bid << new FIX8::SERUM_Data::MDEntryPx(book.bidPrice); // bids
        *nomd_bid << new FIX8::SERUM_Data::MDEntrySize(book.bidSize); // bids
        *nomd_bid << new FIX8::SERUM_Data::SettlDate(update_time_str); // bids
        *nomd_bid << new FIX8::SERUM_Data::MDEntryID("1"); // bids
        *nomd << nomd_bid;

        FIX8::MessageBase *nomd_ask(nomd->create_group());
        *nomd_ask << new FIX8::SERUM_Data::MDEntryType(FIX8::SERUM_Data::MDEntryType_OFFER); // offers
        *nomd_bid << new FIX8::SERUM_Data::MDEntryPx(book.askPrice); // bids
        *nomd_bid << new FIX8::SERUM_Data::MDEntrySize(book.askSize); // bids
        *nomd_bid << new FIX8::SERUM_Data::SettlDate(update_time_str); // bids
        *nomd_bid << new FIX8::SERUM_Data::MDEntryID("2"); // bids
        *nomd << nomd_ask;

        FIX8::MessageBase *nomd_trade(nomd->create_group());
        *nomd_trade << new FIX8::SERUM_Data::MDEntryType(FIX8::SERUM_Data::MDEntryType_TRADE); // trades
        *nomd_bid << new FIX8::SERUM_Data::MDEntryPx(book.lastPrice); // bids
        *nomd_bid << new FIX8::SERUM_Data::MDEntrySize(book.lastSize); // bids
        *nomd_bid << new FIX8::SERUM_Data::SettlDate(update_time_str); // bids
        *nomd_bid << new FIX8::SERUM_Data::MDEntryID("3"); // bids
        *nomd << nomd_trade;
        *mdr << nomd;
    }

    FIX8::Session::send(mdr);
}

void SERUM_Data_session::fullSnapshot(const std::string& reqId, const marketlib::instrument_descr_t& sec_id,
                                  const BrokerModels::DepthSnapshot & depth)
{
    /*
        262 	MDReqID 	Y 	The ID of the Market Data Request that triggered sending this snapshot.
        55 	Symbol 	Y 	The pool expressed as CCY1/CCY2. For example, “DOT/USDT”.
        268 	NoMDEntries 	Y 	The number of market data entries in this message.
                    Begin Repeating Group
        269 	MDEntryType 	Y 	Supported values:

        0	= Bid
        1	= Ask
        2	= Trade


        270 	MDEntryPx 	Y 	The market data entry price.
        271 	MDEntrySize 	Y 	The market data entry volume.
        64 	SettlDate 	Y 	The Settlement date expressed as YYYYMMDD. For example, “19701231”
        278 	MDEntryID 	Y 	The unique ID for this market data.
                    End Repeating Group
     */

    auto *mdr(new FIX8::SERUM_Data::MarketDataSnapshotFullRefresh);
    *mdr   << new FIX8::SERUM_Data::Symbol (sec_id.symbol)
           << new FIX8::SERUM_Data::SecurityExchange (sec_id.engine)
           << new FIX8::SERUM_Data::NoMDEntries(depth.asks.size() + depth.bids.size())
            ;

    {
        FIX8::GroupBase *nomd(mdr->find_group< FIX8::SERUM_Data::MarketDataSnapshotFullRefresh::NoMDEntries >());
        time_t update_time = std::chrono::system_clock::to_time_t(depth.time);
        char *update_time_str = ctime(&update_time);
        for(auto bid: depth.bids) {
            FIX8::MessageBase *nomd_bid(nomd->create_group());
            *nomd_bid << new FIX8::SERUM_Data::MDEntryType(FIX8::SERUM_Data::MDEntryType_BID); // bids
            *nomd_bid << new FIX8::SERUM_Data::MDEntryPx(bid.price); // bids
            *nomd_bid << new FIX8::SERUM_Data::MDEntrySize(bid.volume); // bids
            *nomd_bid << new FIX8::SERUM_Data::SettlDate(update_time_str); // bids
            *nomd_bid << new FIX8::SERUM_Data::MDEntryID(bid.entryId); // bids
            *nomd << nomd_bid;
        }
        for(auto ask: depth.asks) {
            FIX8::MessageBase *nomd_ask(nomd->create_group());
            *nomd_ask << new FIX8::SERUM_Data::MDEntryType(FIX8::SERUM_Data::MDEntryType_OFFER); // offers
            *nomd_ask << new FIX8::SERUM_Data::MDEntryPx(ask.price); // bids
            *nomd_ask << new FIX8::SERUM_Data::MDEntrySize(ask.volume); // bids
            *nomd_ask << new FIX8::SERUM_Data::SettlDate(update_time_str); // bids
            *nomd_ask << new FIX8::SERUM_Data::MDEntryID(ask.entryId); // bids
            *nomd << nomd_ask;
        }
    }

    FIX8::Session::send(mdr);
}

// IBrokerApplication

void SERUM_Data_session::onEvent(const std::string &exchangeName, IBrokerClient::BrokerEvent, const std::string &details)
{
    _logger->Debug((boost::format("> DEX::onEvent => Exchange '%1%'") % exchangeName).str().c_str());
    _logger->Info(details.c_str());
}

void SERUM_Data_session::onReport(const std::string &exchangeName, const std::string &symbol, const BrokerModels::MarketBook&marketBook)
{
    _logger->Info((boost::format("%1%\nAsk(%2%) AskSize(%3%) --- Bid(%4%) BidSize(%5%)")
                  % symbol
                  % marketBook.askPrice
                  % marketBook.askSize
                  % marketBook.bidPrice
                  % marketBook.bidSize).str().c_str());

   // fullSnapshot("123",marketlib::instrument_descr_t{.engine="SERUM",.symbol=symbol},marketBook);
}

void SERUM_Data_session::onReport(const std::string &exchangeName, const std::string &symbol, const BrokerModels::DepthSnapshot&depth)
{
    int count = 7;
    std::ostringstream strs;
    strs << symbol << "\nAsks\n";
    for (auto ask = depth.asks.begin() + count - 1; ask >= depth.asks.begin(); ask--) {
        strs << (*ask).volume << "  " << (*ask).price << std::endl;
    }
    strs << "\nBids\n";
    for (auto bid = depth.bids.begin(); bid < depth.bids.begin() + count; bid++) {
        strs << (*bid).volume << "  " << (*bid).price << std::endl;
    }
    _logger->Info(strs.str().c_str());
    //fullSnapshot("123",marketlib::instrument_descr_t{.engine="SERUM",.symbol=symbol},depth);
}

