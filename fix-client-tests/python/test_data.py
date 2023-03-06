import random
import time
import quickfix as fix
import traceback
from fix_client_wrapper import FixApp
from fix_client_wrapper import BrokerEvent

class Client:
    def __init__(self, config):
        self.price_settings = fix.SessionSettings(config)
        self.price_storeFactory = fix.MemoryStoreFactory()
        self.price_logFactory = fix.FileLogFactory(self.price_settings)
        self.price_application = FixApp('serum')
        self.price_application.event_func = self.on_event
        self.price_application.instruments_func = self.on_instruments
        self.price_application.snapshot_func = self.on_full_snapshot
        self.price_application.incremental_func = self.on_incremental_snapshot
        self.pools = []
        self.instrument1 = {
            'First': "ETH",
            'Second': "USDC",
            'Symbol': "ETH/USDC",
            'SecurityID': "ETH/USDC",
            'SecurityType': "COIN",
            'SecurityExchange': "Serum",
        }

    def on_event(self, data):
        print('! {}-{}'.format(data["broker"], data["event"]))
        if data["event"] is BrokerEvent.SessionLogon:

            # do some logic
            time.sleep(5)
            self.price_application.get_instruments()
            #self.price_application.subscribe(self.instrument1, True, False)
            #self.price_application.subscribe(self.instrument, True, True)

    def on_incremental_snapshot(self, broker, snapshot):
        print("{} | incr for {}, data {}".format(broker, snapshot['pool'], snapshot['data']))

    def on_full_snapshot(self, broker, snapshot):
        print("{} | full for {}, rows {}".format(broker, snapshot['pool'], len(snapshot['data'])))
        for item in snapshot['data']:
            print(item)

    def on_instruments(self, broker, pools):
        self.pools = self.pools + pools

    def subscribePools(self, count, book):
        i = 0
        for pool in self.pools:
            print("POOL {}: {}, Currency: {}".format(pool['SecurityExchange'], pool['Symbol'], pool['Currency']))
            self.price_application.subscribe(pool, True, book)
            i = i + 1
            if i > count:
                break

    def unsubscribePools(self, count, book):
        i = 0
        for pool in self.pools:
            print("POOL {}: {}, Currency: {}".format(pool['SecurityExchange'], pool['Symbol'], pool['Currency']))
            self.price_application.subscribe(pool, False, book)
            i = i + 1
            if i > count:
                break

if __name__ == '__main__':
    try:
        logic = Client('client_stream.cfg')
        price_initiator = fix.SocketInitiator(logic.price_application, logic.price_storeFactory, logic.price_settings,
                                              logic.price_logFactory)
        price_initiator.start()

        message = ''
        while True:
            message = input('enter e to exit the app\n')
            if message == 'st':
                logic.subscribePools(50, False)
            if message == 'ut':
                logic.unsubscribePools(50, False)
            if message == 'sb':
                logic.subscribePools(50, True)
            if message == 'ub':
                logic.unsubscribePools(50, True)
            if message == 'e':
                break

        price_initiator.stop()
        time.sleep(1)

    except Exception as e:
        print("Exception error: '%s'." % e)
        traceback.print_exc()