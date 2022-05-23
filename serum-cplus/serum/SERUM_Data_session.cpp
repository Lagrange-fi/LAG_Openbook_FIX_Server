#include <chrono>
#include <string.h>
#include <functional>

#include "SERUM_Data_session.hpp"

SERUM_Data_session::SERUM_Data_session(const FIX8::F8MetaCntx& ctx,
                                       const FIX8::sender_comp_id& sci,
                                     FIX8::Persister *persist,
                                     FIX8::Logger *logger,
                                     FIX8::Logger *plogger):
        Session(ctx, sci, persist, logger, plogger),
        FIX8::SERUM_Data::FIX8_SERUM_Data_Router(),
        _session_cfg(nullptr){
}

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
    return enforce(seqnum, msg) || msg->process(*this);
}

FIX8::Message *SERUM_Data_session::generate_logon(const unsigned heartbeat_interval, const FIX8::f8String davi)
{
    FIX8::Message* logon = FIX8::Session::generate_logon(heartbeat_interval, davi);
    std::string username;
    if (_session_cfg->GetAttr("username", username))
    {
       // *logon << new FIX8::LMAXTrade::Username(username);
    }
    std::string password;
    if (_session_cfg->GetAttr("password", password))
    {
        //*logon << new FIX8::LMAXTrade::Password(password);
    }
    return logon;
}

bool SERUM_Data_session::handle_logon(const unsigned seqnum, const FIX8::Message *msg)
{
    //std::cout << "LMAXTrade_session: handle_logon\n";
    _logger->LOG_INFO("LMAXTrade_session: handle_logon\n");
   // _is_connected=true;
   // _channel_listener->onEvent(_name, marketlib::channel_info::ci_logon,"");
    return FIX8::Session::handle_logon(seqnum, msg);
}

bool SERUM_Data_session::handle_logout(const unsigned seqnum, const FIX8::Message *msg)
{
    // std::cout << "LMAXTrade_session: handle_logout\n";
    _logger->LOG_INFO("LMAXTrade_session: handle_logout\n");
    //_is_connected=false;
   // _channel_listener->onEvent(_name, marketlib::channel_info::ci_logout,"");
    return FIX8::Session::handle_logon(seqnum, msg);
}

void SERUM_Data_session::modify_outbound(FIX8::Message *msg)
{

    if(_display_debug) {
        // std::ostringstream stream;
        // msg->print(stream);
        // _logger->LOG_DEBUG("---> %.200s\n",  stream.str().c_str());
    }
    return FIX8::Session::modify_outbound(msg);
}

bool SERUM_Data_session::process(const FIX8::f8String& from)
{
    if(_display_debug) {
        if(from.find("35=0") == -1)
            _logger->LOG_INFO("<--- %s\n", from.c_str());
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

    printf("Request pools: %s, %s", reqIdStr.c_str(), reqTypeStr.c_str());
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
    marketlib::market_depth_t depth;
    FIX8::SERUM_Data::MarketDepth  depth_model;
    if(msg->get(depth_model)){
        depth = (marketlib::market_depth_t)depth_model.get();
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
        depth,
        update_type
    };

    if(subscr_type==marketlib::subscription_type::shapshot_update)
        printf("MD subscribe to %s:%s, depth(%d), update type(%d)",
               request.engine.c_str(),
               request.symbol.c_str(),
               request.depth,
               request.update_type);
    else if(subscr_type==marketlib::subscription_type::snapshot_update_disable)
        printf("MD unsubscribe to %s:%s, depth(%d), update type(%d)",
               request.engine.c_str(),
               request.symbol.c_str(),
               request.depth,
               request.update_type);

    return false;
}