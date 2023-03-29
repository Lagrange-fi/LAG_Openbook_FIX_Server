
import random
import time
import quickfix as fix
import quickfix44 as fix44
import traceback
from fix_client_wrapper import FixApp
from fix_client_wrapper import BrokerEvent
from fix_client_wrapper import OrderType, TimeInForce, Side


class Client:
    def __init__(self, config):
        self.settings = fix.SessionSettings(config)
        self.storeFactory = fix.MemoryStoreFactory()
        self.logFactory = fix.FileLogFactory(self.settings)
        self.application = FixApp('serum')
        self.application.event_func = self.on_event
        self.application.snapshot_func = self.on_full_snapshot
        self.application.report_func = self.on_report
        self.application.report_reject_func = self.on_cancel_reject
        self.instrument = {
            'First': "SOL",
            'Second': "USDC",
            'Symbol': "SOL/USDC",
            'SecurityID': "SOL/USDC",
            'SecurityType': "COIN",
            'SecurityExchange': "Serum",
            'Currency': "USDC"
        }
        self.clId = random.randrange(99999)
        self.order = {}

    def on_event(self, data):
        print('! {}-{}'.format(data["broker"], data["event"]))
        if data["event"] is BrokerEvent.SessionLogon:

            # do some logic
            # self.price_application.subscribe(self.instrument, True, False)
            pass

    def on_full_snapshot(self, broker, snapshot):
        print("{} | full for {}, rows {}".format(broker, snapshot['pool'], len(snapshot['data'])))
        for item in snapshot['data']:
            print(item)

    def on_report(self, name, report):
        print("Report(Execution): " + report['OrdStatus'])

    def on_cancel_reject(self, name, report):
        print("Report(CancelReject): " + report['OrdStatus'])

    def send_market(self):
        self.order = {
            'Account': "90874hf7ygf476tgrfgihf874bfjhb",
            'ClOrdID': str(self.clId),
            'Side': Side.Buy,
            'OrderQty': 0.1,
            'OrdType': OrderType.Market,
        }
        self.application.send(self.instrument, self.order)
        self.clId = self.clId + 1

    def send_limit(self):
        self.order = {
            'Account': "90874hf7ygf476tgrfgihf874bfjhb",
            'ClOrdID': str(self.clId),
            'Side': Side.Buy,
            'OrderQty': 0.1,
            'OrdType': OrderType.Limit,
            'Price': 10,
            'TimeInForce': TimeInForce.GoodTillCancel,
        }
        self.application.send(self.instrument, self.order)
        self.clId = self.clId + 1

    def send_limit_executed(self):
        self.order = {
            'Account': "90874hf7ygf476tgrfgihf874bfjhb",
            'ClOrdID': str(self.clId),
            'Side': Side.Buy,
            'OrderQty': 0.1,
            'OrdType': OrderType.Limit,
            'Price': 40,
            'TimeInForce': TimeInForce.GoodTillCancel,
        }

        self.application.send(self.instrument, self.order)
        self.clId = self.clId + 1

    def cancel(self):
        self.application.cancel(self.instrument, str(self.clId), self.order)
        self.clId = self.clId + 1


if __name__ == '__main__':
    try:
        logic = Client('client_trade.cfg')
        price_initiator = fix.SocketInitiator(logic.application, logic.storeFactory, logic.settings,
                                              logic.logFactory)
        price_initiator.start()
        while True:
            message = input('e:exit, m:market order, le:limit executed, l:limit order c:cancel\n')
            if message == "e":
                break
            if message == "m":
                logic.send_market()
            if message == "l":
                logic.send_limit()
            if message == "le":
                logic.send_limit_executed()
            if message == "c":
                logic.cancel()

        price_initiator.stop()
        time.sleep(1)

    except Exception as e:
        print("Exception error: '%s'." % e)
        traceback.print_exc()