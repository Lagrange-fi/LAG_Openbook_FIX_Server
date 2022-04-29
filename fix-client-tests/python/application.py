import random
import time
import quickfix as fix
import quickfix44 as fix44


class BrokerEvent:
    SessionLogon = "SessionLogon"
    SessionLogout = "SessionLogout"
    SessionError = "SessionError"
    MarketDataReject = "MarketDataReject"


class FixApp(fix.Application):
    def __init__(self, name):
        self.my_name = name
        self.__sessionID = None
        self.isConnected = False
        self.event_func = None
        self.snapshot_func = None
        self.incremental_func = None
        self.instrument_func = None
        self.__reqId = 1
        super(FixApp, self).__init__()

    def onCreate(self, sessionID):
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
            if msgType.getValue() == fix.MsgType_SecurityDefinition:
                self.onSecurityDefinition(message, sessionID)
            if msgType.getValue() == fix.MsgType_MarketDataSnapshotFullRefresh:
                self.onFullSnapshot(message, sessionID)
            if msgType.getValue() == fix.MsgType_MarketDataIncrementalRefresh:
                self.onIncrementalSnapshot(message, sessionID)
            if msgType.getValue() == fix.MsgType_MarketDataRequestReject:
                self.onMarketDataReject(message, sessionID)
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

    def onFullSnapshot(self, message, sessionID):
        symbol = fix.Symbol()
        securityID = fix.SecurityID()
        securityExchange = fix.SecurityExchange()
        snapshot = {
            'SecurityID': message.getField(securityID).getString(),
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
            snapshot['data'].append({
                'MDEntryType': group.getField(mdEntryType).getString(),
                'MDEntryID': group.getField(1023),
                'MDEntrySize': group.getField(mdEntryPx).getString(),
                'MDEntryPx': group.getField(mdEntrySize).getString(),
                # 'MDEntryLevel': mdEntryLevel.getString(),
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

    def onInstrument(self, message, sessionID):
        pass

class Client:
    def __init__(self, config):
        self.price_settings = fix.SessionSettings(config)
        self.price_storeFactory = fix.MemoryStoreFactory()
        self.price_logFactory = fix.FileLogFactory(self.price_settings)
        self.price_application = FixApp('serum')
        self.price_application.event_func = self.on_event
        self.price_application.instrument_func = self.on_instrument
        self.price_application.snapshot_func = self.on_full_snapshot
        self.price_application.incremental_func = self.on_incremental_snapshot
        self.instrument = {
            'First': "BTC",
            'Second': "USDT",
            'Symbol': "BTCUSDT",
            'SecurityID': "BTCUSDT",
            'SecurityType': "COIN",
            'SecurityExchange': "SERUM",
        }

    def on_event(self, data):
        print('! {}-{}'.format(data["broker"], data["event"]))
        if data["event"] is BrokerEvent.SessionLogon:
            self.price_application.get_instruments()
            self.start_some_logic()

    def on_instrument(self, broker, instr):
        print("{}: {}".format(broker, instr['Symbol']))

    def on_full_snapshot(self, broker, snapshot):
        print("{} | full for {}, rows {}".format(broker, snapshot['pool'], len(snapshot['data'])))

    def on_incremental_snapshot(self, broker, snapshot):
        print("{} | incr for {}, data {}".format(broker, snapshot['pool'], snapshot['data']))

    def start_some_logic(self):
        print("-----------------------------")
        print("Subscribe-Unsubscribe test")
        self.price_application.subscribe(self.instrument, 1, True, True)
        time.sleep(10)
        self.price_application.subscribe(self.instrument, 1, False)


if __name__ == '__main__':
    try:
        logic = Client('stream.cfg')
        price_initiator = fix.SocketInitiator(logic.price_application, logic.price_storeFactory, logic.price_settings,
                                              logic.price_logFactory)
        price_initiator.start()
        while True:
            pass
    except Exception as e:
        print("Exception error: '%s'." % e)
