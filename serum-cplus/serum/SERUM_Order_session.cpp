#include <functional>
#include <ctime>
#include <thread>
#include <memory>
#include <functional>

#include <SerumDEX/SerumMarket/Market.hpp>
#include <SerumDEX/SerumMarket/models.hpp>
#include <SerumDEX/SerumTrade.h>

#include "SERUM_Order_session.hpp"

#include "ConsoleLogger.h"

const char* TRADE_CONN_NAME="Serum";

#define PUBKEY "5ejVgdkJkgwptewzs5CLYdm6vJxpEL47sErAcnom6i8A"
#define SECRETKEY "4Vi55eAt4aCfPGrbQKWjESK9UkKvb8AU7jwrzj8ajeA8dbPXEXwB4L1uhkdDEzoZ8bWhJXoRFXcE7aeeve4seARp"

SERUM_Order_session::SERUM_Order_session(const FIX8::F8MetaCntx& ctx,
                    const FIX8::sender_comp_id& sci,
                    FIX8::Persister *persist,
                    FIX8::Logger *slogger,
                    FIX8::Logger *plogger):
        Session(ctx, sci, persist, slogger, plogger),
        FIX8::SERUM_Order::FIX8_SERUM_Order_Router(),
        _logger(new ConsoleLogger)
{
    _logger->Debug((boost::format("OSession | construct ")).str().c_str());
    /*
    auto market = SerumMarket(
            PUBKEY,
            SECRETKEY,
            "https://nd-664-169-151.p2pify.com/a89ccd991de179587a0b8e3356409a9b",
            pools,
            trade_channel,
            [](const string& name, const Instrument& inst, const string& info){
                std::cout << name << " || " << inst.symbol << " || " + info;},

            [](const string& name, const execution_report_t& execution_report) {
                std::cout << "Order Update" << std::endl;
                std::cout << "id:" << execution_report.clId << std::endl;
                std::cout << "status" << str_state(execution_report.state) << endl;
            },
            "Market_1"
    );
    */
}

void SERUM_Order_session::setupOpenbook(const std::shared_ptr < IPoolsRequester >& pools, std::shared_ptr < IListener >  trade_channel )
{
    SerumMarket* market = new SerumMarket(
        PUBKEY,
        SECRETKEY,
        "https://nd-664-169-151.p2pify.com/a89ccd991de179587a0b8e3356409a9b",
        _logger,
        pools,
        trade_channel,
        std::bind( &SERUM_Order_session::reportCallback, this, std::placeholders::_1, std::placeholders::_2),
        "Market_1"
     );
    _market = std::shared_ptr<SerumMarket> (market);
}

void SERUM_Order_session::reportCallback(const std::string& name, const marketlib::execution_report_t& report)
{
    if(report.type == marketlib::rt_partial_fill||report.type == marketlib::rt_fill||report.type == marketlib::rt_fill_trade)
    {
        sendExecutionReport(report);
    }
    else if(report.type == marketlib::rt_cancel_rejected)
    {
        sendCancelRejectReport(report.clId, report.text);
    }
    else
    {
        sendReport(report);
    }
}


SERUM_Order_session::~SERUM_Order_session()
{
    _logger->Debug((boost::format("OSession | destruct ")).str().c_str());
}

const std::string& SERUM_Order_session::sess_id()
{
    return this->get_sid().get_id();
}

// FIX8::Session implementation
bool SERUM_Order_session::handle_application(const unsigned seqnum, const FIX8::Message *&msg)
{
    _logger->Debug((boost::format("OSession | handle_application ")).str().c_str());
    if(enforce(seqnum,msg)){
        _logger->Error("OSession | enforce checking problem ");
        return false;
    }
    try{return msg->process(*this) ;}
    catch(std::exception& ex)
    {
        _logger->Error((boost::format("OSession | handle_application exception (%1%) ") % ex.what()).str().c_str());
        //detach(msg);
        //msg = 0;
        throw;
    }
}

bool SERUM_Order_session::handle_logon(const unsigned seqnum, const FIX8::Message *msg)
{
    _logger->Debug((boost::format("OSession | handle_logon ")).str().c_str());
    return FIX8::Session::handle_logon(seqnum, msg);
}

bool SERUM_Order_session::handle_logout(const unsigned seqnum, const FIX8::Message *msg)
{
    _logger->Debug((boost::format("OSession | handle_logout ")).str().c_str());
    try {
        _logger->Info((boost::format("OSession | Serum session  stop ")).str().c_str());
        this->stop();
        // _control |= shutdown;
    }
    catch(std::exception& ex)
    {
        _logger->Error((boost::format("OSession | Serum session stop exception(%1%)")% ex.what()).str().c_str());
    }
    return FIX8::Session::handle_logout(seqnum, msg);
}

void SERUM_Order_session::modify_outbound(FIX8::Message *msg)
{
    FIX8::Session::modify_outbound(msg);
}

bool SERUM_Order_session::process(const FIX8::f8String& from)
{
    if(from.find("35=0") == -1) {
        // for all except heartbeat
        _logger->Info((boost::format("OSession | <-- %1%") % from).str().c_str());
    }
    return FIX8::Session::process(from);
}

FIX8::Message *SERUM_Order_session::generate_logon(const unsigned heartbeat_interval, const FIX8::f8String davi)
{
    FIX8::Message* logon = FIX8::Session::generate_logon(heartbeat_interval, davi);
    return logon;
}

///////////// FIX8::SERUM_Data::FIX8_SERUM_Data_Router implementation /////////////////


bool SERUM_Order_session::operator() (const class FIX8::SERUM_Order::NewOrderSingle *msg) const
{
    /*
     1	Account	Y	Owner, depend on DEX
    */
    marketlib::order_t order;
    FIX8::SERUM_Order::Account acct;
    if(msg->get(acct) && acct.is_valid()){
        order.owner = acct.get();
    }

    /*
    11 	ClOrdID 	Y 	The client-assigned unique ID for the order. Notice that the maximum size of this tag is 58 bytes.
   */
    FIX8::SERUM_Order::ClOrdID clid;
    if(msg->get(clid) && clid.is_valid()){
        order.clId = clid.get();
    }

    /*
    55 	Symbol 	Y 	The pool name expressed as CCY1/CCY2. For example, “DOT/USDT”.
   */
    FIX8::SERUM_Order::Symbol symbol;
    if(msg->get(symbol) && symbol.is_valid()){
        order.secId = symbol.get();
    }

    /*
    15 	Currency 	Y 	The fixed currency of the trade, either CCY1 or CCY2.
   */
    FIX8::SERUM_Order::Currency currency;
    if(msg->get(currency) && currency.is_valid()){
        order.currency = currency.get();
    }

    /*
   54 	Side 	Y 	Supported values:
  */
    FIX8::SERUM_Order::Side side;
    if(msg->get(side) && side.is_valid()){
        order.side = (marketlib::order_side_t)side.get();
    }

    /*
     60 	TransactTime 	Y 	The UTCTimestamp when the order was sent by the client.
   */
    FIX8::SERUM_Order::TransactTime time;
    if(msg->get(time)){
        order.init_time = time.get().secs();
    }

    /*
      38 	OrderQty 	Y 	The order quantity in the base currency (15).
     */
    FIX8::SERUM_Order::OrderQty qty;
    if(msg->get(qty)){
        order.original_qty = qty.get();
    }

    /*
     40 	OrdType 	Y 	The order type.
    */
    FIX8::SERUM_Order::OrdType type;
    if(msg->get(type) && type.is_valid()){
        order.type = (marketlib::order_type_t)type.get();
    }

    /*
      59 	TimeInForce 	Y 	Supported values:
    */
    FIX8::SERUM_Order::TimeInForce tif;
    if(msg->get(tif) && tif.is_valid()){
        order.tif = (marketlib::time_in_force_t)tif.get();
    }

    /*
       44 	Price 	N* 	The price at which the limit order should be executed.
                                                                              */
    FIX8::SERUM_Order::Price price;
    if(msg->get(price)){
        order.price = price.get();
    }

    auto session = const_cast<SERUM_Order_session*>(this);
    marketlib::instrument_descr_t pool;// {.engine=TRADE_CONN_NAME,.symbol=symbol.get()};
    pool.engine = TRADE_CONN_NAME;
    pool.symbol=symbol.get();

    // order validation
    if(order.clId.empty() || order.owner.empty() ||  order.secId.empty() || order.currency.empty() || order.original_qty <= 0
              ||!(order.type==marketlib::order_type_t::ot_Market||order.type==marketlib::order_type_t::ot_Limit))
    {
        marketlib::execution_report_t report;
        report.clId = order.clId;
        report.type = marketlib::report_type_t::rt_rejected;
        report.state = marketlib::order_state_t::ost_Rejected;
        report.text = "wrong input parameters;";
        session->sendReport(report);
        return true;
        //| <-- 8=FIX.4.49=17735=D34=249=Lagrange-fi-client-ord52=20220617-15:18:52.00056=Lagrange-fi-ord1=90874hf7ygf476tgrfgihf874bfjhb11=1000015=USDC38=140=154=155=ETHUSDC60=20220617-15:18:5210=235
    }

    _logger->Debug((boost::format("Ord. Session | Execute %1% %2% order id(%3%), qty(%4%), price(%5%) ")
        %(char)order.side % (char)order.type % order.clId % order.original_qty % order.price).str().c_str());

    session->sendReport(marketlib::execution_report_t(order.clId,marketlib::order_state_t::ost_Pending,marketlib::report_type_t::rt_pending_new));
    order = _market->send_new_order(pool, order);
    return true;
}

bool SERUM_Order_session::operator() (const class FIX8::SERUM_Order::OrderCancelRequest *msg) const
{
    /*
     41 	OrigClOrdID 	Y 	The unique ID of the last non-cancelled order assigned by the client.
        Notice that the maximum size of this tag is 58 bytes.
        37 	OrderId 	N 	The unique ID of the order, as returned to the client in the Execution Report in the OrderId field (37).
        11 	ClOrdID 	Y 	The unique ID of the cancel request as assigned by the Client.

        Note: This is not the ClOrdID of the order being cancelled, but rather an identifier of the cancel request.
        55 	Symbol 	Y 	The same as in the original order.
        54 	Side 	Y 	The same as in the original order.
        60 	TransactTime 	Y 	UTCTimestamp of the request from the client.
        38 	OrderQty 	N 	The quantity being cancelled.
     */

    /*
    11 	ClOrdID 	Y 	The client-assigned unique ID for the order. Notice that the maximum size of this tag is 58 bytes.
   */
    FIX8::SERUM_Order::ClOrdID clid;
    std::string clid_str;
    if(msg->get(clid) && clid.is_valid()){
        clid_str = clid.get();
    }

    /*
    37 	OrderId 	N 	The unique ID of the order, as returned to the client in the Execution Report in the OrderId field (37).
   */
  /*    FIX8::SERUM_Order::OrderID orderId;
    std::string orderId_str;
    if(msg->get(orderId) && orderId.is_valid()){
        orderId_str = orderId.get();
    }*/


   //rigClOrdID 	Y 	The unique ID of the last non-cancelled order assigned by the client.
    FIX8::SERUM_Order::OrigClOrdID origClOrdID;
    std::string orig_clid_str;
    if(msg->get(origClOrdID) && origClOrdID.is_valid()){
        orig_clid_str = origClOrdID.get();
    }


   // 55 	Symbol 	Y 	The pool name expressed as CCY1/CCY2. For example, “DOT/USDT”.

   std::string pair;
    FIX8::SERUM_Order::Symbol symbol;
    if(msg->get(symbol) && symbol.is_valid()){
        pair = symbol.get();
    }

    /*
   54 	Side 	Y 	Supported values:
    */
   /* FIX8::SERUM_Order::Side side;
    if(msg->get(side) && side.is_valid()){
        order.side = (marketlib::order_side_t)side.get();
    }*/

    /*
     60 	TransactTime 	Y 	The UTCTimestamp when the order was sent by the client.
    */
    /* FIX8::SERUM_Order::TransactTime time;
    if(msg->get(time)){
        order.init_time = time.get().secs();
    }*/

    /*
      38 	OrderQty 	Y 	The order quantity in the base currency (15).
     */
    /* FIX8::SERUM_Order::OrderQty qty;
    if(msg->get(qty)){
        order.original_qty = qty.get();
    }*/


    if(pair.empty() || orig_clid_str.empty())
    {
        auto session = const_cast<SERUM_Order_session*>(this);
        session->sendCancelRejectReport(orig_clid_str,  "wrong input parameters");
        return true;
    }

    marketlib::instrument_descr_t pool;
    pool.engine = TRADE_CONN_NAME;
    pool.symbol=symbol.get();

    _logger->Debug((boost::format("Ord. Session | Cancelling order %1%") % orig_clid_str).str().c_str());
    _market->cancel_order(pool, orig_clid_str);
    return true;
}

void SERUM_Order_session::sendReport(const marketlib::execution_report_t& report)
{
    /*
    37 	OrderId 	N 	The unique ID of the order, as returned to the client in the Execution Report in the OrderId field (37).
    11 	ClOrdID 	Y 	The unique ID of the order assigned by the client. This ID would have been the new ClOrdID.
    41 	OrigClOrdID 	N 	The ID of the order assigned by the client that could not be modified or canceled.
    39 	OrdStatus 	Y 	The order status after the Cancel Reject.
    58 	Text 	N 	Message
 */
    if(report.clId.empty())
    {
        _logger->Error(boost::format("OSession | sendReport, clId is empty").str().c_str());
    }

    auto *mdr(new FIX8::SERUM_Order::ExecutionReport);
    *mdr    << new FIX8::SERUM_Order::ClOrdID(report.clId)
            << new FIX8::SERUM_Order::OrdStatus(report.state)
            << new FIX8::SERUM_Order::ExecType (report.type);

    if(!report.exchId.empty())
        *mdr    << new FIX8::SERUM_Order::OrderID (report.exchId);
    //if(!report.origClId.empty())
    //    *mdr    << new FIX8::SERUM_Order::OrigClOrdID (report.origClId);
    if(!report.text.empty())
        *mdr    << new FIX8::SERUM_Order::Text (report.text);

    _logger->Info((boost::format("OSession | --> %1%,%2%, clid(%3%)") % (char)report.state % (char)report.type % report.clId).str().c_str());

    FIX8::Session::send(mdr);
}

void SERUM_Order_session::sendCancelRejectReport( const std::string &clId, const std::string &text)
{
    /*
    37 	OrderId 	N 	The unique ID of the order, as returned to the client in the Execution Report in the OrderId field (37).
    11 	ClOrdID 	Y 	The unique ID of the order assigned by the client. This ID would have been the new ClOrdID.

    When CxlRejResponseTo is “Order Cancel
    Request”, this is identical to the ClOrdID (11) in the Order Cancel Request (35=F).
    When CxlRejResponseTo is “Order Cancel/Replace Request”, this is identical to the ClOrdID (11) in the Order Cancel/Replace Request (35=F).
    41 	OrigClOrdID 	N 	The ID of the order assigned by the client that could not be modified or canceled.

    39 	OrdStatus 	Y 	The order status after the Cancel Reject.

    Note: If CxlRejReason is “Unknown order”, the
    OrdStatus should be Rejected (8).
    434 	CxlRejResponseTo 	Y 	The type of request that has been rejected.
    102 	CxlRejReason 	N 	The reason for the rejection.
    58 	Text 	N 	Message
   */

    auto *mdr(new FIX8::SERUM_Order::OrderCancelReject);
    *mdr    << new FIX8::SERUM_Order::ClOrdID(clId)
            << new FIX8::SERUM_Order::OrdStatus(marketlib::order_state_t::ost_Canceled);

    //if(!report.exchId.empty())
    //*mdr    << new FIX8::SERUM_Order::OrderID (report.exchId);
    //if(!report.origClId.empty())
    //    *mdr    << new FIX8::SERUM_Order::OrigClOrdID (report.origClId);
    *mdr    << new FIX8::SERUM_Order::CxlRejReason (1);//  unknown order
    *mdr    << new FIX8::SERUM_Order::CxlRejResponseTo (FIX8::SERUM_Order::CxlRejResponseTo_ORDER_CANCEL_REQUEST);
    if(!text.empty())
        *mdr    << new FIX8::SERUM_Order::Text (text);

    /*
    <field name='OrderID' required='N' />
   <field name='ExecID' required='N' />
   <field name='Side' required='N' />
   <field name='LeavesQty' required='N' />
   <field name='CumQty' required='N' />
   <field name='AvgPx' required='N' />
    */
    // change XML and delete  these  tags
    //*mdr    << new FIX8::SERUM_Order::OrderID ("123");
    //*mdr    << new FIX8::SERUM_Order::OrigClOrdID (clId);

   _logger->Info((boost::format("OSession | --> Cancel_Rejected clid(%1%)") % clId).str().c_str());
    FIX8::Session::send(mdr);
}

void SERUM_Order_session::sendExecutionReport(const marketlib::execution_report_t & report)
{
    /*s
    37 	OrderId 	N 	The unique ID for the order assigned by the server.
    11 	ClOrdID 	Y 	The unique ID of the order assigned by the client as specified in the ClOrdID (11) field in a New Order Single(35=D), Order Cancel Request (35=F)
    17 	ExecID 	    N 	The unique ID of the execution.
    150 ExecType 	Y 	The type of execution that was reported.
    39 	OrdStatus 	Y 	The current status of the order.
    32 	LastQty 	N 	The quantity bought/sold on this (last) fill. (*) Required if ExecType (150) = Trade (‘F’).
    31 	LastPx 	    N 	Price of this (last) fill.

    151 LeavesQty 	N 	The quantity remaining to be executed in the order specified in the units of the Currency (15).
    14 	CumQty 	N 	The executed amount of the order specified in the units of the Currency (15).
    60 	TransactTime 	N 	The UTCTimestamp of the execution time on the server.

    55 	Symbol    	N 	Currency pair symbol expressed as CCY1/CCY2. For example, “DOT/USDT”.
    54 	Side 	    N 	Supported values:
    59 	TimeInForce 	N 	Supported values:
    38 	OrderQty 	N 	The Quantity of the order specified in the units of the Currency (15).
    44 	Price 	    N* 	The price of the order.
 */

    if(report.clId.empty())
    {
        _logger->Error(boost::format("OSession | sendExecutionReport, clId is empty").str().c_str());
    }
    auto *mdr(new FIX8::SERUM_Order::ExecutionReport);
    *mdr    << new FIX8::SERUM_Order::ClOrdID(report.clId)
            << new FIX8::SERUM_Order::OrdStatus(report.state)
            << new FIX8::SERUM_Order::ExecType(report.type)
            << new FIX8::SERUM_Order::LastQty(report.lastShares)
            << new FIX8::SERUM_Order::LastPx(report.lastPx);

    if(!report.exchId.empty())
        *mdr    << new FIX8::SERUM_Order::OrderID (report.exchId);
    if(!report.tradeId.empty())
        *mdr    << new FIX8::SERUM_Order::ExecID (report.tradeId);



    _logger->Info((boost::format("OSession | --> Trade, clid(%1%), lastPx(%2%), lastShared(%3%)") % report.clId % report.lastPx % report.lastShares).str().c_str());
    FIX8::Session::send(mdr);
}



