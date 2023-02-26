#include <functional>
#include <ctime>
#include <thread>
#include <boost/format.hpp>

#include "SERUM_Order_sandbox_session.hpp"

#include "ConsoleLogger.h"

const char* SAND_TRADE_CONN_NAME="Serum";

SERUM_Order_sandbox_session::SERUM_Order_sandbox_session(const FIX8::F8MetaCntx& ctx,
                    const FIX8::sender_comp_id& sci,
                    FIX8::Persister *persist,
                    FIX8::Logger *slogger,
                    FIX8::Logger *plogger):
        Session(ctx, sci, persist, slogger, plogger),
        FIX8::SERUM_Order::FIX8_SERUM_Order_Router(),
        _logger(new ConsoleLogger)
{
    _logger->Debug((boost::format("OSession | construct ")).str().c_str());
}

SERUM_Order_sandbox_session::~SERUM_Order_sandbox_session()
{
    _logger->Debug((boost::format("OSession | destruct ")).str().c_str());
}

const std::string& SERUM_Order_sandbox_session::sess_id()
{
    return this->get_sid().get_id();
}

// FIX8::Session implementation
bool SERUM_Order_sandbox_session::handle_application(const unsigned seqnum, const FIX8::Message *&msg)
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

bool SERUM_Order_sandbox_session::handle_logon(const unsigned seqnum, const FIX8::Message *msg)
{
    _logger->Debug((boost::format("OSession | handle_logon ")).str().c_str());
   /* try {
        _logger->Info((boost::format("OSession | Serum DEX start ")).str().c_str());
        _client->start();
    }
    catch(std::exception& ex)
    {
        _logger->Error((boost::format("OSession | Serum DEX start exception(%1%)")% ex.what()).str().c_str());
    }*/
    return FIX8::Session::handle_logon(seqnum, msg);
}

bool SERUM_Order_sandbox_session::handle_logout(const unsigned seqnum, const FIX8::Message *msg)
{
    _logger->Debug((boost::format("OSession | handle_logout ")).str().c_str());
    try {
        _logger->Info((boost::format("OSession | Serum DEX stop ")).str().c_str());
        //_client->stop();
        //while(_client->isConnected())
        //    std::this_thread::sleep_for(std::chrono::milliseconds(200));
        this->stop();
        // _control |= shutdown;
    }
    catch(std::exception& ex)
    {
        _logger->Error((boost::format("OSession | Serum DEX stop exception(%1%)")% ex.what()).str().c_str());
    }
    return FIX8::Session::handle_logout(seqnum, msg);
}

void SERUM_Order_sandbox_session::modify_outbound(FIX8::Message *msg)
{
    FIX8::Session::modify_outbound(msg);
}

bool SERUM_Order_sandbox_session::process(const FIX8::f8String& from)
{
    if(from.find("35=0") == -1) {
        // for all except heartbeat
        _logger->Info((boost::format("OSession | <-- %1%") % from).str().c_str());
    }
    return FIX8::Session::process(from);
}

FIX8::Message *SERUM_Order_sandbox_session::generate_logon(const unsigned heartbeat_interval, const FIX8::f8String davi)
{
    FIX8::Message* logon = FIX8::Session::generate_logon(heartbeat_interval, davi);
    return logon;
}

// FIX8::SERUM_Data::FIX8_SERUM_Data_Router implementation
bool SERUM_Order_sandbox_session::operator() (const class FIX8::SERUM_Order::NewOrderSingle *msg) const
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
        order.price = qty.get();
    }

    auto session = const_cast<SERUM_Order_sandbox_session*>(this);
    marketlib::instrument_descr_t pool {.engine=SAND_TRADE_CONN_NAME,.sec_id=symbol.get(),.symbol=symbol.get()};
    // order validation
    if(order.clId.empty() || order.owner.empty() || order.secId.empty() || order.currency.empty() || order.original_qty <= 0
              ||!(order.type==marketlib::order_type_t::ot_Market||order.type==marketlib::order_type_t::ot_Limit))
    {
        session->sendReport(order.clId, marketlib::report_type_t::rt_rejected,
                            marketlib::order_state_t::ost_Rejected,"", "", "wrong input parameters");
        return true;
        //| <-- 8=FIX.4.49=17735=D34=249=Lagrange-fi-client-ord52=20220617-15:18:52.00056=Lagrange-fi-ord1=90874hf7ygf476tgrfgihf874bfjhb11=1000015=USDC38=140=154=155=ETHUSDC60=20220617-15:18:5210=235
    }

    order.exchId = "ex" + order.clId;
    static int tradeId = 10000;
    if(order.type==marketlib::order_type_t::ot_Market)
    {
        session->sendReport(order.clId, marketlib::report_type_t::rt_new,marketlib::order_state_t::ost_New,order.exchId);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        session->sendExecutionReport(std::to_string(tradeId++), order.clId, marketlib::report_type_t::rt_fill_trade,
                                     marketlib::order_state_t::ost_Filled, order.exchId, order.price, order.original_qty);
    }
    else if(order.type==marketlib::order_type_t::ot_Limit)
    {
        if(_orders.find(order.clId) == _orders.end()) {
            _orders[order.clId] = order;
            _logger->Debug((boost::format("OSession | OrderBook Add,  count=%1%") % _orders.size()).str().c_str());

            session->sendReport(order.clId, marketlib::report_type_t::rt_new,marketlib::order_state_t::ost_New,order.exchId);
        }
        else {
            session->sendReport(order.clId, marketlib::report_type_t::rt_rejected,
                                marketlib::order_state_t::ost_Rejected,"", "", "order client id already exist");
        }
    }

    return true;
}

bool SERUM_Order_sandbox_session::operator() (const class FIX8::SERUM_Order::OrderCancelRequest *msg) const
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
    FIX8::SERUM_Order::OrderID orderId;
    std::string orderId_str;
    if(msg->get(orderId) && orderId.is_valid()){
        orderId_str = orderId.get();
    }

    /*
   OrigClOrdID 	Y 	The unique ID of the last non-cancelled order assigned by the client.
   */
    FIX8::SERUM_Order::OrigClOrdID origClOrdID;
    std::string orig_clid_str;
    if(msg->get(origClOrdID) && origClOrdID.is_valid()){
        orig_clid_str = origClOrdID.get();
    }

    /*
    55 	Symbol 	Y 	The pool name expressed as CCY1/CCY2. For example, “DOT/USDT”.
   */
    /*FIX8::SERUM_Order::Symbol symbol;
    if(msg->get(symbol) && symbol.is_valid()){
        order.secId = symbol.get();
    }*/

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

    auto session = const_cast<SERUM_Order_sandbox_session*>(this);
    auto orderIt = _orders.find(orig_clid_str);
    if(orderIt != _orders.end()) {
        session->sendReport(orig_clid_str, marketlib::report_type_t::rt_canceled,
                            marketlib::order_state_t::ost_Canceled,orderIt->second.exchId);
        _orders.erase(orderIt);
        _logger->Debug((boost::format("OSession | OrderBook delete,  count=%1%") % _orders.size()).str().c_str());
    }
    else {
        session->sendCancelRejectReport(orig_clid_str, "unknown client id");
    }
    return true;
}

void SERUM_Order_sandbox_session::sendReport(const std::string&clId,marketlib::report_type_t type,marketlib::order_state_t state,
                const std::string&exchId,  const std::string&origClId, const std::string&text)
{
    /*
    37 	OrderId 	N 	The unique ID of the order, as returned to the client in the Execution Report in the OrderId field (37).
    11 	ClOrdID 	Y 	The unique ID of the order assigned by the client. This ID would have been the new ClOrdID.
    41 	OrigClOrdID 	N 	The ID of the order assigned by the client that could not be modified or canceled.
    39 	OrdStatus 	Y 	The order status after the Cancel Reject.
    58 	Text 	N 	Message
 */
    auto *mdr(new FIX8::SERUM_Order::ExecutionReport);
    *mdr    << new FIX8::SERUM_Order::ClOrdID(clId)
            << new FIX8::SERUM_Order::OrdStatus(state)
            << new FIX8::SERUM_Order::ExecType (type);

    if(!exchId.empty())
        *mdr    << new FIX8::SERUM_Order::OrderID (exchId);
    if(!origClId.empty())
        *mdr    << new FIX8::SERUM_Order::OrigClOrdID (origClId);
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
    *mdr    << new FIX8::SERUM_Order::ExecID ("deleteme");
    *mdr    << new FIX8::SERUM_Order::Side ('1');
    *mdr    << new FIX8::SERUM_Order::LeavesQty ("0");
    *mdr    << new FIX8::SERUM_Order::CumQty ("0");
    *mdr    << new FIX8::SERUM_Order::AvgPx ("0.1");
    if(exchId.empty())
        *mdr    << new FIX8::SERUM_Order::OrderID ("123");
    _logger->Info((boost::format("OSession | --> %1%, clid(%2%)") % (char)state % clId).str().c_str());

    FIX8::Session::send(mdr);
}

void SERUM_Order_sandbox_session::sendCancelRejectReport( const std::string &clId, const std::string &text)
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
    *mdr    << new FIX8::SERUM_Order::OrderID ("123");
    *mdr    << new FIX8::SERUM_Order::OrigClOrdID (clId);

   _logger->Info((boost::format("OSession | --> Cancel_Rejected clid(%1%)") % clId).str().c_str());
    FIX8::Session::send(mdr);
}

void SERUM_Order_sandbox_session::sendExecutionReport(const std::string &tradeId, const std::string &clId,
                marketlib::report_type_t type, marketlib::order_state_t state, const std::string &exchId,double lastPx, double lastShares)
{
    /*s
    37 	OrderId 	N 	The unique ID for the order assigned by the server.
    11 	ClOrdID 	Y 	The unique ID of the order assigned by the client as specified in the ClOrdID (11) field in a New Order Single(35=D), Order Cancel Request (35=F)
    17 	ExecID 	    N 	The unique ID of the execution.
    150 ExecType 	Y 	The type of execution that was reported.
    39 	OrdStatus 	Y 	The current status of the order.
    32 	LastQty 	Y 	The quantity bought/sold on this (last) fill. (*) Required if ExecType (150) = Trade (‘F’).
    31 	LastPx 	    Y 	Price of this (last) fill.

    151 LeavesQty 	N 	The quantity remaining to be executed in the order specified in the units of the Currency (15).
    14 	CumQty 	N 	The executed amount of the order specified in the units of the Currency (15).
    60 	TransactTime 	N 	The UTCTimestamp of the execution time on the server.

    55 	Symbol    	N 	Currency pair symbol expressed as CCY1/CCY2. For example, “DOT/USDT”.
    54 	Side 	    N 	Supported values:
    59 	TimeInForce 	N 	Supported values:
    38 	OrderQty 	N 	The Quantity of the order specified in the units of the Currency (15).
    44 	Price 	    N* 	The price of the order.
 */

    auto *mdr(new FIX8::SERUM_Order::ExecutionReport);
    *mdr    << new FIX8::SERUM_Order::ClOrdID(clId)
            << new FIX8::SERUM_Order::OrdStatus(state)
            << new FIX8::SERUM_Order::ExecType(type)
            << new FIX8::SERUM_Order::LastQty(lastShares)
            << new FIX8::SERUM_Order::LastPx(lastPx);

    if(!exchId.empty())
        *mdr    << new FIX8::SERUM_Order::OrderID (exchId);
    if(!tradeId.empty())
        *mdr    << new FIX8::SERUM_Order::ExecID (tradeId);

    /*
    <field name='OrderID' required='N' />
   <field name='ExecID' required='N' />
   <field name='Side' required='N' />
   <field name='LeavesQty' required='N' />
   <field name='CumQty' required='N' />
   <field name='AvgPx' required='N' />
    */
    // change XML and delete  these  tags
    //*mdr    << new FIX8::SERUM_Order::Side ('1');
    //*mdr    << new FIX8::SERUM_Order::LeavesQty ("0");
    //*mdr    << new FIX8::SERUM_Order::CumQty ("0");
    // *mdr    << new FIX8::SERUM_Order::AvgPx ("0.1");
    //if(exchId.empty())
   //     *mdr    << new FIX8::SERUM_Order::OrderID ("123");

    _logger->Info((boost::format("OSession | --> Trade, clid(%1%), lastPx(%2%), lastShared(%3%)") % clId % lastPx % lastShares).str().c_str());
    FIX8::Session::send(mdr);
}



