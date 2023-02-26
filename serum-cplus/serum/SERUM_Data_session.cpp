#include <functional>
#include <ctime>
#include <list>
#include <boost/format.hpp>

#include "SERUM_Data_session.hpp"

//#include <SerumDEX/SerumMD.h>
//#include <SerumDEX/PoolRequester/PoolsRequester.h>
#include "ConsoleLogger.h"

const char* CONN_NAME="Serum";

SERUM_Data_session::SERUM_Data_session(const FIX8::F8MetaCntx& ctx,
                                       const FIX8::sender_comp_id& sci,
                                     FIX8::Persister *persist,
                                     FIX8::Logger *slogger,
                                     FIX8::Logger *plogger):
        Session(ctx, sci, persist, slogger, plogger),
        FIX8::SERUM_Data::FIX8_SERUM_Data_Router(),
        _logger(new ConsoleLogger),
        _client(nullptr)
        //_client( std::shared_ptr <IBrokerClient>(new SerumMD(_logger,_settings, std::make_shared< PoolsRequester >( _logger, _settings ), [](const std::string &exchangeName, marketlib::broker_event, const std::string &details) {})) )
{
    _logger->Debug((boost::format("Session | construct ")).str().c_str());
    _clientId = std::to_string((long)this);
    //slogger->send((boost::format("Session | construct ")).str());
    //plogger->send((boost::format("Session | construct ")).str());
}

SERUM_Data_session::~SERUM_Data_session()
{
    _logger->Debug((boost::format("Session | destruct ")).str().c_str());
}


const std::string& SERUM_Data_session::sess_id()
{
    return this->get_sid().get_id();
}

bool SERUM_Data_session::handle_application(const unsigned seqnum, const FIX8::Message *&msg)
{
    _logger->Debug((boost::format("Session | handle_application ")).str().c_str());
    if(enforce(seqnum,msg)){
        _logger->Error("Session | enforce checking problem ");
        return false;
    }
    try{return msg->process(*this) ;}
    catch(std::exception& ex)
    {
        _logger->Error((boost::format("Session | handle_application exception (%1%) ") % ex.what()).str().c_str());
        //detach(msg);
        //msg = 0;
        throw;
    }
    //return enforce(seqnum, msg) || msg->process(*this);
}

FIX8::Message *SERUM_Data_session::generate_logon(const unsigned heartbeat_interval, const FIX8::f8String davi)
{
    // _logger->Debug((boost::format("Session | generate_logon ")).str().c_str());
    FIX8::Message* logon = FIX8::Session::generate_logon(heartbeat_interval, davi);
    return logon;
}

bool SERUM_Data_session::handle_logon(const unsigned seqnum, const FIX8::Message *msg)
{
    _logger->Debug((boost::format("Session | handle_logon ")).str().c_str());
   /* try {
       _logger->Info((boost::format("Session | Serum DEX start ")).str().c_str());
       _client->start();
   }
   catch(std::exception& ex)
   {
       _logger->Error((boost::format("Session | Serum DEX start exception(%1%)")% ex.what()).str().c_str());
   }*/
   return FIX8::Session::handle_logon(seqnum, msg);
}

bool SERUM_Data_session::handle_logout(const unsigned seqnum, const FIX8::Message *msg)
{
    _logger->Debug((boost::format("Session | handle_logout ")).str().c_str());
    try {
        _logger->Info((boost::format("Session | Serum DEX stop ")).str().c_str());
        // _client->stop();
        //  while(_client->isConnected())
        //  std::this_thread::sleep_for(std::chrono::milliseconds(200));
        _client->unsubscribeForClientId(_clientId);

        // _control |= shutdown;
        this->stop();
    }
    catch(std::exception& ex)
    {
        _logger->Error((boost::format("Session | Serum DEX stop exception(%1%)")% ex.what()).str().c_str());
    }
    return FIX8::Session::handle_logout(seqnum, msg);
}

void SERUM_Data_session::modify_outbound(FIX8::Message *msg)
{
    FIX8::Session::modify_outbound(msg);
}


bool SERUM_Data_session::process(const FIX8::f8String& from)
{
    if(from.find("35=0") == -1) {
        // for all except heartbeat
        _logger->Info((boost::format("Session | <-- %1%") % from).str().c_str());
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

    _logger->Info((boost::format("Session | SecurityListRequest, SecurityReqID (%1%)") % reqIdStr).str().c_str());

    auto pools = _client->getInstruments();

    auto* _sess = const_cast<SERUM_Data_session*>(this);
    _sess->securityList(reqIdStr,marketlib::security_request_result_t::srr_valid,pools);

    return true;
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
    marketlib::market_depth_t depth;
    FIX8::SERUM_Data::MarketDepth  market_depth;
    if(msg->get(market_depth)){
        depth = (marketlib::market_depth_t)market_depth.get();
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
        55 	Symbol 	Y 	The pool symbol expressed as CCY1/CCY2. For example, “DOT/USDT”.s
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
        CONN_NAME,
        symbol.get(),
        depth,
        update_type
    };

    marketlib::instrument_descr_t pool {.engine=CONN_NAME,.sec_id=symbol.get(),.symbol=symbol.get()};
    if(subscr_type==marketlib::subscription_type::shapshot_update)
    {
        try{
            if(depth == marketlib::market_depth_t::top){
                _logger->Info((boost::format("Session | MD subscribe TOP to %1% : %2%, depth(%3%), update type(%4%)")
                               % request.engine % request.symbol % request.depth % request.update_type).str().c_str());

                //_client->subscribe(pool,depth,_clientId,[] (const std::string &exch, const std::string &pair, const std::any &data) {});


                _client->subscribe(pool,depth,_clientId,[this, reqIdStr, pool] (const std::string &exch, const std::string &pair, const std::any &data) {
                   auto marketBook = std::any_cast<BrokerModels::MarketBook>(data);
                    _logger->Debug((boost::format("Session | --> 35=W, %1%, Ask(%2%) AskSize(%3%) --- Bid(%4%) BidSize(%5%)")
                                    % pair % marketBook.askPrice% marketBook.askSize% marketBook.bidPrice% marketBook.bidSize).str().c_str());
                   auto* _sess = const_cast<SERUM_Data_session*>(this);
                    _sess->fullSnapshot(reqIdStr, pool, marketBook);
               });
            }

            else if(depth == marketlib::market_depth_t::full){
                _logger->Info((boost::format("Session | MD subscribe FULL to %1% : %2%, depth(%3%), update type(%4%)")
                               % request.engine % request.symbol % request.depth % request.update_type).str().c_str());
                _client->subscribe(pool, depth, _clientId,
                        [this, reqIdStr, pool] (const std::string &exch, const std::string &pair, const std::any &data) {
                            auto marketDepth = std::any_cast<BrokerModels::DepthSnapshot>(data);
                            _logger->Debug((boost::format("Session | --> 35=W, %1% , count = %2%")
                                            % pair % (marketDepth.bids.size() + marketDepth.asks.size()) ).str().c_str());
                            auto* _sess = const_cast<SERUM_Data_session*>(this);
                            _sess->fullSnapshot(reqIdStr, pool, marketDepth);
                        }
                );
            }
        }
        catch(std::exception& ex)
        {
            _logger->Error((boost::format("Session | !!! Subscribe to %1%, exception %2%")% symbol % ex.what()).str().c_str());
            _logger->Error((boost::format("Session | Incorrect Subscribe/Unsubscribe to %1% : %2%, depth(%3%), update type(%4%)")
                           % request.engine % request.symbol % request.depth % request.update_type).str().c_str());
            auto* _sess = const_cast<SERUM_Data_session*>(this);
            _sess->marketReject(reqIdStr, marketlib::ord_rej_reason::rr_broker);
        }
    }
    else if(subscr_type==marketlib::subscription_type::snapshot_update_disable)
    {
        _logger->Info((boost::format("Session | MD unsubscribe to %1% : %2%, depth(%3%), update type(%4%)")
                       % request.engine % request.symbol % request.depth % request.update_type).str().c_str());
        try
        {
            _client->unsubscribe(pool,depth,_clientId);
        }
        catch(std::exception& ex)
        {
            _logger->Error((boost::format("Session | !!! Unsubscribe to %1%, exception %2%")
                           % symbol % ex.what()).str().c_str());;
        }
    }
    else {
        _logger->Info((boost::format("Session | Incorrect Subscribe/Unsubscribe to %1% : %2%, depth(%3%), update type(%4%)")
                       % request.engine % request.symbol % request.depth % request.update_type).str().c_str());
        auto* _sess = const_cast<SERUM_Data_session*>(this);
        _sess->marketReject(reqIdStr, marketlib::ord_rej_reason::rr_broker);
    }

    return true;
}

void SERUM_Data_session::securityList(const std::string &reqId, marketlib::security_request_result_t result,
                                      const std::list<marketlib::instrument_descr_t>& pools)
{

    // test security list  response//
    /*
      {
        "name": "BTC/USDC",
        "baseCurrency": "BTC",
        "quoteCurrency": "USDC",
        "version": 3,
        "address": "A8YFbxQYFVqKZaoYJLLUVcQiWP7G2MeEgW5wsAQgMvFw",
        "programId": "9xQeWvG816bUx9EPjHmaT23yvVM2ZWbrrpZb9PusVFin",
        "baseMintAddress": "9n4nbM75f5Ui33ZbPYXn59EwSgE8CGsHtAeTH5YFeJ9E",
        "quoteMintAddress": "EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v",
        "tickSize": 0.1,
        "minOrderSize": 0.0001,
        "deprecated": false
      }

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
    std::list<marketlib::instrument_descr_t>::const_iterator it = pools.begin();
    while(it != pools.end())
    {
        auto *mdr(new FIX8::SERUM_Data::SecurityList);

        *mdr    << new FIX8::SERUM_Data::SecurityReqID(reqId)
                << new FIX8::SERUM_Data::SecurityResponseID("resp" + reqId)
                << new FIX8::SERUM_Data::SecurityRequestResult(result);

        int offset = 0;
        FIX8::GroupBase *noin(mdr->find_group<FIX8::SERUM_Data::SecurityList::NoRelatedSym>());
        for (; offset <100 && it != pools.end() ; ++it, offset++) {
            FIX8::MessageBase *noin_sym(noin->create_group());
            *noin_sym << new FIX8::SERUM_Data::SecurityExchange(it->engine)
                      << new FIX8::SERUM_Data::Symbol(it->symbol)
                      << new FIX8::SERUM_Data::Currency(it->quote_currency);
            *noin << noin_sym;
        }
        *mdr   << new FIX8::SERUM_Data::NoRelatedSym(offset) ;
        *mdr << noin;

        _logger->Info( (boost::format("Session | --> 35=y, count = %1%") % (int)offset ).str().c_str() );

        FIX8::Session::send(mdr);
    }

    auto *mdr(new FIX8::SERUM_Data::SecurityList);

    *mdr    << new FIX8::SERUM_Data::SecurityReqID(reqId)
            << new FIX8::SERUM_Data::SecurityResponseID("resp" + reqId)
            << new FIX8::SERUM_Data::SecurityRequestResult(result)
            << new FIX8::SERUM_Data::NoRelatedSym((int)0) ;
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

    time_t update_time = std::chrono::system_clock::to_time_t(book.time);
    tm *_tm = gmtime(&update_time);
    std::chrono::system_clock::duration duration = book.time.time_since_epoch();
    auto milli_total = std::chrono::duration_cast<std::chrono::milliseconds> (duration).count();
    char buff[64];
    sprintf(buff, "%.2d:%.2d:%.2d.%.3ld", _tm->tm_hour,_tm->tm_min, _tm->tm_sec, milli_total % 1000);

     auto *mdr(new FIX8::SERUM_Data::MarketDataSnapshotFullRefresh);
     *mdr   << new FIX8::SERUM_Data::Symbol (sec_id.symbol)
            << new FIX8::SERUM_Data::SecurityExchange (sec_id.engine)
            << new FIX8::SERUM_Data::NoMDEntries(2)
            << new FIX8::SERUM_Data::MDReqID (reqId)
            ;
    {
        FIX8::GroupBase *nomd(mdr->find_group< FIX8::SERUM_Data::MarketDataSnapshotFullRefresh::NoMDEntries >());
        FIX8::MessageBase *nomd_bid(nomd->create_group());
        *nomd_bid << new FIX8::SERUM_Data::MDEntryType(FIX8::SERUM_Data::MDEntryType_BID) // bids
                  << new FIX8::SERUM_Data::MDEntryPx(book.bidPrice) // bids
                  << new FIX8::SERUM_Data::MDEntrySize(book.bidSize) // bids
                  << new FIX8::SERUM_Data::MDEntryDate((tm&)(*_tm))
                  << new FIX8::SERUM_Data::MDEntryTime(buff)
                  ;
        *nomd << nomd_bid;

        FIX8::MessageBase *nomd_ask(nomd->create_group());
        *nomd_ask << new FIX8::SERUM_Data::MDEntryType(FIX8::SERUM_Data::MDEntryType_OFFER) // offers
                  << new FIX8::SERUM_Data::MDEntryPx(book.askPrice) // bids
                  << new FIX8::SERUM_Data::MDEntrySize(book.askSize) // bids
                  << new FIX8::SERUM_Data::MDEntryDate((tm&)(*_tm))
                  << new FIX8::SERUM_Data::MDEntryTime(buff)
                  ;
        *nomd << nomd_ask;
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
           << new FIX8::SERUM_Data::MDReqID (reqId)
            ;

    {
        FIX8::GroupBase *nomd(mdr->find_group< FIX8::SERUM_Data::MarketDataSnapshotFullRefresh::NoMDEntries >());

        time_t update_time = std::chrono::system_clock::to_time_t(depth.time);
        tm *_tm = gmtime(&update_time);
        std::chrono::system_clock::duration duration = depth.time.time_since_epoch();
        auto milli_total = std::chrono::duration_cast<std::chrono::milliseconds> (duration).count();
        char buff[64];
        sprintf(buff, "%.2d:%.2d:%.2d.%.3ld", _tm->tm_hour,_tm->tm_min, _tm->tm_sec, milli_total % 1000);

        for(auto bid: depth.bids) {
            FIX8::MessageBase *nomd_bid(nomd->create_group());
            *nomd_bid << new FIX8::SERUM_Data::MDEntryType(FIX8::SERUM_Data::MDEntryType_BID) // bids
                      << new FIX8::SERUM_Data::MDEntryPx(bid.price) // bids
                      << new FIX8::SERUM_Data::MDEntrySize(bid.volume) // bids
                      << new FIX8::SERUM_Data::MDEntryDate((tm&)(*_tm))
                      << new FIX8::SERUM_Data::MDEntryTime(buff)
                      ;
            *nomd << nomd_bid;
        }
        for(auto ask: depth.asks) {
            FIX8::MessageBase *nomd_ask(nomd->create_group());
            *nomd_ask << new FIX8::SERUM_Data::MDEntryType(FIX8::SERUM_Data::MDEntryType_OFFER) // offers
                      << new FIX8::SERUM_Data::MDEntryPx(ask.price) // bids
                      << new FIX8::SERUM_Data::MDEntrySize(ask.volume) // bids
                      << new FIX8::SERUM_Data::MDEntryDate((tm&)(*_tm))
                      << new FIX8::SERUM_Data::MDEntryTime(buff)
                      ;
            *nomd << nomd_ask;
        }
    }

    FIX8::Session::send(mdr);
}
