import time
import quickfix as fix
import quickfix44 as fix44
import traceback


class BrokerEvent:
    SessionLogon = "SessionLogon"
    SessionLogout = "SessionLogout"
    SessionError = "SessionError"
    MarketDataReject = "MarketDataReject"


class Side:
    Buy = '1'
    Sell = '2'


class SubscriptionModel:
    TopBook = "TopBook"
    FullBook = "FullBook"


class OrderType:
    Market = '1'
    Limit = '2'
    StopLimit = '3'
    Stop = '4'


class OrderStatus:
    PartiallyFilled = '1'
    Filled = '2'
    New = '0'
    Canceled = '4'
    Replaced = '5'
    Rejected = '8'
    PendingCancel = '6'
    PendingNew = 'A'
    PendingReplace = 'E'


class TimeInForce:
    Day = '0'
    GoodTillCancel = '1'
    ImmediateOrCancel = '3'
    FillOrKill = '4'


class InstrumentType:
    STK = "STK"
    FUT = "FUT"
    OPT = "OPT"
    BIN = "BIN"


class FixApp(fix.Application):
    def __init__(self, name):
        self.my_name = name
        self.__sessionID = None
        self.isConnected = False
        self.event_func = None
        self.snapshot_func = None
        self.incremental_func = None
        self.instruments_func = None
        self.report_func = None
        self.report_reject_func = None
        self.__reqId = 1
        super(FixApp, self).__init__()

    def onCreate(self, sessionID):
        print("onCreate:")
        return

    def onLogon(self, sessionID):
        self.__sessionID = sessionID
        self.isConnected = True
        data = {
            'broker': self.my_name,
            'event': BrokerEvent.SessionLogon,
            'description': "",
        }
        self.event_func(data)

    def onLogout(self, sessionID):
        self.isConnected = False
        data = {
            'broker': self.my_name,
            'event': BrokerEvent.SessionLogout,
            'description': "",
        }
        self.event_func(data)
        return

    def toAdmin(self, message, sessionID):
        return

    def fromAdmin(self, message, sessionID):
        print("<--%s" % message.toString())
        return

    def toApp(self, message, sessionID):
        print("-->%s" % message.toString())
        return

    def fromApp(self, message, sessionID):
        print("<--%s" % message.toString())
        msgType = fix.MsgType()
        message.getHeader().getField(msgType)
        try:
            if msgType.getValue() == fix.MsgType_SecurityList:
                self.onInstruments(message, sessionID)
            if msgType.getValue() == fix.MsgType_MarketDataSnapshotFullRefresh:
                self.onFullSnapshot(message, sessionID)
            if msgType.getValue() == fix.MsgType_MarketDataIncrementalRefresh:
                self.onIncrementalSnapshot(message, sessionID)
            if msgType.getValue() == fix.MsgType_MarketDataRequestReject:
                self.onMarketDataReject(message, sessionID)
            if msgType.getValue() == fix.MsgType_ExecutionReport:
                self.onReport(message, sessionID)
            if msgType.getValue() == fix.MsgType_OrderCancelReject:
                self.onCancelReject(message, sessionID)
        except Exception as exc:
            print("fromApp: " + exc + " for " + message)

    def get_instruments(self):
        sdr = fix44.SecurityListRequest()
        sdr.setField(fix.SecurityReqID(str(self.__reqId)))
        sdr.setField(fix.SecurityListRequestType(4))
        fix.Session.sendToTarget(sdr, self.__sessionID)
        self.__reqId = self.__reqId + 1

    def subscribe(self, instrument, subscr=True, full_book=False, incrementals=False):
        request = fix44.MarketDataRequest()
        request.setField(fix.MDReqID(str(self.__reqId)))
        request.setField(fix.MDUpdateType(5))
        if subscr:
            request.setField(fix.SubscriptionRequestType('1'))
        else:
            request.setField(fix.SubscriptionRequestType('2'))

        if full_book:
            request.setField(fix.MarketDepth(0))
        else:
            request.setField(fix.MarketDepth(1))

        request.setField(fix.NoMDEntryTypes(3))
        bid_group = fix44.MarketDataRequest.NoMDEntryTypes()
        bid_group.setField(fix.MDEntryType(fix.MDEntryType_BID))
        request.addGroup(bid_group)
        ask_group = fix44.MarketDataRequest.NoMDEntryTypes()
        ask_group.setField(fix.MDEntryType(fix.MDEntryType_OFFER))
        request.addGroup(ask_group)
        trade_group = fix44.MarketDataRequest.NoMDEntryTypes()
        trade_group.setField(fix.MDEntryType(fix.MDEntryType_TRADE))
        request.addGroup(trade_group)

        symbol = fix44.MarketDataRequest.NoRelatedSym()
        symbol.setField(fix.Symbol(instrument.get("Symbol") or ""))
        symbol.setField(fix.SecurityID(instrument.get("SecurityID") or ""))
        symbol.setField(fix.SecurityType(instrument.get("SecurityType") or ""))
        symbol.setField(fix.SecurityExchange(instrument.get("SecurityExchange") or ""))
        request.addGroup(symbol)
        fix.Session.sendToTarget(request, self.__sessionID)
        self.__reqId = self.__reqId + 1

    def send(self, instrument, order):
        request = fix44.NewOrderSingle()
        request.setField(fix.Account(order['Account']))
        request.setField(fix.ClOrdID(order['ClOrdID']))
        request.setField(fix.Symbol(instrument['Symbol']))
        request.setField(fix.Currency(instrument['Currency']))
        request.setField(fix.Side(order['Side']))
        request.setField(fix.TransactTime())
        request.setField(fix.OrderQty(order["OrderQty"]))
        request.setField(fix.OrdType(order["OrdType"]))
        if order["OrdType"] == OrderType.Limit:
            request.setField(fix.Price(order["Price"]))
        if order["OrdType"] == OrderType.Limit:
            request.setField(fix.TimeInForce(order["TimeInForce"]))

        fix.Session.sendToTarget(request, self.__sessionID)
        pass

    def cancel(self, instrument, newOrdID, order):
        request = fix44.OrderCancelRequest()
        request.setField(fix.ClOrdID(newOrdID))
        request.setField(fix.OrigClOrdID(order["ClOrdID"]))
        request.setField(fix.Symbol(instrument["Symbol"]))
        request.setField(fix.Side(order["Side"]))
        request.setField(fix.TransactTime())

        fix.Session.sendToTarget(request, self.__sessionID)
        pass

    def onFullSnapshot(self, message, sessionID):
        symbol = fix.Symbol()
        securityExchange = fix.SecurityExchange()
        snapshot = {
            'pool': message.getField(symbol).getString(),
            'SecurityExchange': message.getField(securityExchange).getString() if message.isSetField(
                securityExchange) else "",
            'data': [],
        }
        noMdEntries = fix.NoMDEntries()
        count = int(message.getField(noMdEntries).getString())
        for item in range(1, count + 1):
            group = fix44.MarketDataSnapshotFullRefresh.NoMDEntries()
            message.getGroup(item, group)
            mdEntryType = fix.MDEntryType()
            mdEntryPx = fix.MDEntryPx()
            mdEntrySize = fix.MDEntrySize()
            mdEntryDate = fix.MDEntryDate()
            mdEntryTime = fix.MDEntryTime()
            snapshot['data'].append({
                'MDEntryType': group.getField(mdEntryType).getString(),
                'MDEntrySize': group.getField(mdEntryPx).getString(),
                'MDEntryPx': group.getField(mdEntrySize).getString(),
                'MDEntryDate': group.getField(mdEntryDate).getString(),
                'MDEntryTime': group.getField(mdEntryTime).getString(),
            })
        self.snapshot_func(self.my_name, snapshot)

    def onIncrementalSnapshot(self, message, sessionID):
        noMdEntries = fix.NoMDEntries()
        securityIDStr = ""
        count = int(message.getField(noMdEntries).getString())
        for item in range(1, count + 1):
            group = fix44.MarketDataIncrementalRefresh.NoMDEntries()
            message.getGroup(item, group)
            mdEntryType = fix.MDEntryType()
            mdEntryPx = fix.MDEntryPx()
            mdEntrySize = fix.MDEntrySize()
            action = fix.MDUpdateAction()
            securityID = fix.SecurityID()
            if group.isSetField(securityID):
                securityIDStr = group.getField(securityID).getString()
            snapshot = {
                'MDUpdateAction': group.getField(action).getString(),
                'MDEntryType': group.getField(mdEntryType).getString(),
                'MDEntryID': group.getField(1023),
                'SecurityID': securityIDStr,
                'MDEntrySize': group.getField(mdEntrySize).getString() if group.isSetField(mdEntrySize) else "",
                'MDEntryPx': group.getField(mdEntryPx).getString() if group.isSetField(mdEntryPx) else ""
            }
            self.incremental_func(self.my_name, snapshot)

    def onMarketDataReject(self, message, sessionID):
        rejReason = fix.MDReqRejReason()
        securityID = fix.SecurityID()
        text = fix.Text()
        data = {
            'broker': self.my_name,
            'event': BrokerEvent.MarketDataReject,
            'SecurityID': message.getField(securityID).getString() if message.isSetField(securityID) else "",
            'description': message.getField(text).getString() if message.isSetField(text) else "",
            'reason': message.getField(rejReason).getString() if message.isSetField(rejReason) else "",
        }
        self.event_func(data)
        pass

    def onInstruments(self, message, sessionID):
        req_result = fix.SecurityRequestResult()
        result = message.getField(req_result).getString()
        pools = []
        if result == "0":
            symbolGroup = fix.NoRelatedSym()
            count = int(message.getField(symbolGroup).getString())
            for item in range(1, count + 1):
                group = fix44.SecurityList.NoRelatedSym()
                message.getGroup(item, group)

                symbol = fix.Symbol()
                exchange = fix.SecurityExchange()
                currency = fix.Currency()
                pools.append({
                    'Symbol': group.getField(symbol).getString(),
                    'SecurityExchange':  group.getField(exchange).getString(),
                    'Currency': group.getField(currency).getString(),
                })

        self.instruments_func(self.my_name, pools)

    def onReport(self, message, sessionID):
        '''
        37 	OrderId 	N 	The unique ID for the order assigned by the server.
        11 	ClOrdID 	Y 	The unique ID of the order assigned by the client as specified in the ClOrdID (11) field in a New Order Single(35=D), Order Cancel Request (35=F)
        41 	OrigClOrdID 	N* 	The unique client assigned ID of the original order that was subject to an Order Cancel Request (35=F)
        103 OrdRejReason 	N 	Code to identify reason for order rejection.
        17 	ExecID 	N 	The unique ID of the execution.
        150 ExecType 	Y 	The type of execution that was reported.
        39 	OrdStatus 	Y 	The current status of the order.
        55 	Symbol 	N 	Currency pair symbol expressed as CCY1/CCY2. For example, “DOT/USDT”.
        54 	Side 	N 	Supported values:
        59 	TimeInForce 	N 	Supported values:
        38 	OrderQty 	N 	The Quantity of the order specified in the units of the Currency (15).
        44 	Price 	N* 	The price of the order.
        15 	Currency 	N 	The fixed currency of the trade.
        32 	LastQty 	N* 	The quantity bought/sold on this (last) fill. (*) Required if ExecType (150) = Trade (‘F’).
        31 	LastPx 	N* 	Price of this (last) fill.
        151 LeavesQty 	N 	The quantity remaining to be executed in the order specified in the units of the Currency (15).
        14 	CumQty 	N 	The executed amount of the order specified in the units of the Currency (15).
        6 	AvgPx 	N* 	The average price at which the order was executed.
        60 	TransactTime 	N 	The UTCTimestamp of the execution time on the server.
        58 	Text 	N 	Message
        '''

        clId = fix.ClOrdID()
        origClId = fix.OrigClOrdID()  # N
        orderId = fix.OrderID()  # N
        rej_reason = fix.OrdRejReason()
        exec_id = fix.ExecID()
        exec_type = fix.ExecType()
        ord_status = fix.OrdStatus()
        symbol = fix.Symbol()
        side = fix.Side()
        last_px = fix.LastPx()  # N
        last_shares = fix.LastShares()  # N
        avgPx = fix.AvgPx()
        cum_qty = fix.CumQty()  # N
        leaves_qty = fix.LeavesQty()  #
        text = fix.Text()  # N

        report = {
            'ClOrdID': message.getField(clId).getString(),
            'OrigClOrdID': message.getField(origClId).getString() if message.isSetField(origClId) else "",
            'OrderID': message.getField(orderId).getString() if message.isSetField(orderId) else "",
            'OrdRejReason': message.getField(rej_reason).getString() if message.isSetField(rej_reason) else "",
            'ExecID': message.getField(exec_id).getString() if message.isSetField(exec_id) else "",
            'ExecType': message.getField(exec_type).getString(),
            'OrdStatus': message.getField(ord_status).getString(),
            'Symbol': message.getField(symbol).getString() if message.isSetField(symbol) else "",
            'Side': message.getField(side).getString() if message.isSetField(side) else "",
            'LastPx': message.getField(last_px).getString() if message.isSetField(last_px) else "",
            'LastShares': message.getField(last_shares).getString() if message.isSetField(last_shares) else "",
            'AvgPx': message.getField(avgPx).getString() if message.isSetField(avgPx) else "",
            'CumQty': message.getField(cum_qty).getString() if message.isSetField(cum_qty) else "",
            'LeavesQty': message.getField(leaves_qty).getString() if message.isSetField(leaves_qty) else "",
            'Text': message.getField(text).getString() if message.isSetField(text) else "",
        }
        self.report_func(self.my_name, report)

    def onCancelReject(self, message, sessionID):

        '''

        37 	OrderId 	Y 	The unique ID of the order, as returned to the client in the Execution Report in the OrderId field (37).

        If the rejection occurred because of unknown order, this tag is set to NONE.
        11 	ClOrdID 	Y 	The unique ID of the order assigned by the client. This ID would have been the new ClOrdID.

        When CxlRejResponseTo is “Order Cancel
        Request”, this is identical to the ClOrdID (11) in the Order Cancel Request (35=F).

        When CxlRejResponseTo is “Order Cancel/Replace Request”, this is identical to the ClOrdID (11) in the Order Cancel/Replace Request (35=F).
        41 	OrigClOrdID 	Y 	The ID of the order assigned by the client that could not be modified or canceled.
        39 	OrdStatus 	Y 	The order status after the Cancel Reject.

        Note: If CxlRejReason is “Unknown order”, the
        OrdStatus should be Rejected (8).
        434 	CxlRejResponseTo 	Y 	The type of request that has been rejected.

        Supported values:

        1	= Order Cancel Request
        2	= Order Cancel/Replace Request
        102 	CxlRejReason 	N 	The reason for the rejection.
        58 	Text 	N 	Message
        '''

        clId = fix.ClOrdID()
        status = fix.OrdStatus()
        exec_type = fix.ExecType()
        text = fix.Text()
        rej_response = fix.CxlRejResponseTo()
        rej_reason = fix.CxlRejReason()
        report = {
            'OrdClID': message.getField(clId).getString(),
            'CxlRejResponseTo': message.getField(rej_response).getString(),
            'OrdStatus': message.getField(status).getString(),
            'Text': message.getField(text).getString() if message.isSetField(text) else "",
        }
        self.report_reject_func(self.my_name, report)