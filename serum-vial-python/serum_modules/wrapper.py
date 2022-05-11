import websocket
from .models import *
import json
import threading
from sortedcontainers import SortedList    

from random import randint
from base58 import b58decode as decode

from pyserum.connection import get_live_markets, get_token_mints
from pyserum.connection import conn
from pyserum.market import Market
from pyserum.enums import OrderType, Side


from solana.rpc.types import TxOpts
from solana.keypair import Keypair
from solana.publickey import PublicKey
from solana.rpc.api import Client
from solana.rpc.types import TokenAccountOpts




WSS_URL = 'wss://api.serum-vial.dev/v1/ws'
REST_URL = 'https://solana-api.projectserum.com'

class Wrapper:
    def __init__(self, keypair, logger, message_event, information_event):
        self.__owner = Keypair(decode(keypair)[:32])
        self.__message_event = message_event
        self.__information_event = information_event
        self.__logger = logger
        self.__channel_list = []
        self.__snapshots_map = {}
        self.__ws = SocketObj(WSS_URL, self.__logger, self.__m_event, self.__i_event)
        self.__all_mint_address = get_token_mints()
        self.__all_market_address = get_live_markets()
        self.__markets = {}
        
        
    def start(self):
        self.__ws.start()
        
        
    def subscribe(self, channel: Channels, instr: Instrument):
        if SubscribedChannel(channel, instr.market) in self.__channel_list:
            return
        self.__ws.send(json.dumps(
            {
                'op': Ops.Subscribe,
                'channel': channel,
                'markets': [instr.market]
            }
        ))
        
    
    def unsubscribe(self, channel: Channels, instr: Instrument):
        if SubscribedChannel(channel, instr.market) in self.__channel_list:
            self.__ws.send(json.dumps(
                {
                    'op': Ops.Unsubscribe,
                    'channel': channel,
                    'markets': [instr.market]
                }
            ))
    
    
    def __m_event(self, event, msg = None):
        match event:
            case MessageType.Level1.Quote:
                self.__message_event(Channels.Level1, msg)
            case MessageType.Level2.Snapshot:
                self.__snapshots_map[msg['market']] = msg
                self.__message_event(Channels.Level2, self.__snapshots_map[msg['market']])
            case MessageType.Level2.Update:
                snpt_ask = self.__snapshots_map[msg['market']]['asks']
                
                snpt_ask = [ask for ask in snpt_ask 
                            if ask[0] not in [
                                ask[0] for ask in msg['asks'] 
                                if ask[1] == 0
                            ]
                ] + [ask for ask in msg['asks'] if ask[1] != 0]
                
                
                snpt_bid = self.__snapshots_map[msg['market']]['bids']
                
                snpt_bid = [bid for bid in snpt_bid 
                            if bid[0] not in [
                                bid[0] for bid in msg['bids'] 
                                if bid[1] == 0
                            ]
                ] + [bid for bid in msg['bids'] if bid[1] != 0]
                
                self.__snapshots_map[msg['market']]['asks'] = list(SortedList(snpt_ask))
                self.__snapshots_map[msg['market']]['bids'] = list(SortedList(snpt_bid))[::-1]
                
                self.__message_event(Channels.Level2, self.__snapshots_map[msg['market']])
                
            case _:
                self.__message_event(Channels.Level3, msg)
                
                
    def __i_event(self, event, msg = None):
        match event:
            case MessageType.Event.Subscribed:
                self.__channel_list.append(msg)
            case MessageType.Event.Unsubscribed:
                self.__channel_list.remove(msg)
        
        self.__information_event(event, msg)
        
        
    def __get_new_id(self):
        return randint(0, 0xffffffffffffffff)
    
    
    def send_new_order(self, order: Order):
        if order.client_id == 0:
            order.client_id = self.__get_new_id()
        
        if order.market not in self.__markets or not self.__markets[order.market]._conn.is_connected:
            self.create_new_market_connector(order.market)
        
        try:
            payer = [i.address for i in self.__all_mint_address if i.name == order.second][0]
            
            if order.side == Side.BUY:
                payer = PublicKey(decode(
                    Client(REST_URL).get_token_accounts_by_owner(
                        self.__owner.public_key, 
                        TokenAccountOpts(payer)
                    )['result']['value'][0]['pubkey']
                ))
            
            tx = self.__markets[order.market].place_order(
                payer=payer,
                owner=self.__owner,
                side=order.side,
                order_type=OrderType.LIMIT,
                limit_price=order.price,
                max_quantity=order.amount,
                client_id=order.client_id,
                opts = TxOpts(skip_preflight=True)
            )
            
            self.__logger.info(f'order sent: side {order.side} price {order.price} size {order.amount} id {order.client_id} tx {tx}')
        except Exception as e:
            self.__logger.error(e)
            
        return order
            
            
    def cancel_order(self, my_order: Order):
        if my_order.market not in self.__markets or not self.__markets[my_order.market]._conn.is_connected:
            self.create_new_market_connector(my_order.market)
        
        oredrs = []
        if my_order.side == Side.BUY:
            orders = self.__markets[my_order.market].load_bids()
        else: 
            orders = self.__markets[my_order.market].load_asks()
            
        orders = [order for order in orders if order.client_id == my_order.client_id]
        if len(orders) == 0:
            self.__logger.info(f'order not found')
            return

        order = orders[0]
        tx = self.__markets[my_order.market].cancel_order(self.__owner, order, opts=TxOpts(skip_preflight=True))
        self.__logger.info(f'cancelled order: side {my_order.side} price {order.info.price} size {order.info.size} id {order.client_id} tx {tx}')
        
        
    def create_new_market_connector(self, market):
        try:
            self.__markets[market] = Market.load(
                conn(REST_URL), 
                [i.address for i in self.__all_market_address if i.name == market][0]
            )
        except Exception as e:
            self.__logger.error(e)
        
        
class SocketObj(object):
    def __init__(self, url, logger, message_event, information_event):
        self.__message_event = message_event
        self.__information_event = information_event
        self.__logger = logger
        self.__thread_warning = None
        self.__url = url
        self.__isStarted = False
        self.__isConnected = False
        websocket.enableTrace(False)
        

    def start(self):
        self.__ws = websocket.WebSocketApp(self.__url, 
                                           on_open=self.__on_open,
                                           on_message=self.__on_message,
                                           on_error=self.__on_error,
                                           on_close=self.__on_close)
        
        wst = threading.Thread(target=self.__ws.run_forever, daemon=True)
        wst.start()
        self.__isStarted = True

        # self.__logger.debug('connection started')

        self.__thread_warning = threading.Thread(name='WarningLoop', daemon=True)
        self.__thread_warning.start()


    def stop(self):
        self.__ws.close()
        # self.__logger.debug('stopped')


    def isStarted(self):
        return self.__isStarted


    def send(self, msg):
        try:
            self.__ws.send(msg)
        except:
            pass
        
    def __on_error(self, ws, error):
        self.__logger.error(f'<-- Error {error}')
        self.__information_event(BrokerEvent.SessionError, error)


    def __on_open(self, ws):
        self.__information_event(BrokerEvent.SessionLogon)
        # self.__logger.info('<-- connection open')
        self.__isConnected = True


    def __on_close(self, ws, close_status_code, close_msg):
        self.__isConnected = False
        self.__information_event(BrokerEvent.SessionLogout)
        # self.__logger.info(f'<-- connection close  code--{close_status_code}  msg--{close_msg}')
        

    def __on_message(self, ws, message):
        dump = json.loads(message)
        
        match dump['type']:
            case MessageType.Level1.Quote:
                self.__parse_quote(dump)
            case MessageType.Level2.Snapshot:
                self.__parse_snapshot_lvl2(dump)
            case MessageType.Level2.Update:
                self.__parse_update_lvl2(dump)
            case MessageType.Level3.Snapshot:
                self.__parse_snapshot_lvl3(dump)
            case MessageType.Level3.Open:
                self.__parse_open_lv3(dump)
            case MessageType.Level3.Change:
                self.__parse_change_lv3(dump)
            case MessageType.Level3.Fill:
                self.__parse_fill_lv3(dump)
            case MessageType.Level3.Done:
                self.__parse_done_lv3(dump)
            
            case MessageType.Event.Subscribed: 
                self.__information_event(
                    MessageType.Event.Subscribed, 
                    SubscribedChannel(dump['channel'], dump['markets'][0])
                )
            case MessageType.Event.Unsubscribed: 
                self.__information_event(
                    MessageType.Event.Unsubscribed, 
                    SubscribedChannel(dump['channel'], dump['markets'][0])
                )
                
                
    def __parse_quote(self, dump):
        self.__message_event(MessageType.Level1.Quote,
            {
                'market': dump['market'],
                'bestAsk': [float(dump['bestAsk'][0]), float(dump['bestAsk'][1])],
                'bestBid': [float(dump['bestBid'][0]), float(dump['bestBid'][1])]
            }
        )
        
        
    def __parse_snapshot_lvl2(self, dump):
        self.__message_event(MessageType.Level2.Snapshot, 
            {
                'market': dump['market'],
                'asks': [[float(ask[0]), float(ask[1])] for ask in dump['asks']],
                'bids': [[float(bid[0]), float(bid[1])] for bid in dump['bids']]
            }
        )
    
    
    def __parse_update_lvl2(self, dump):
        self.__message_event(MessageType.Level2.Update, 
            {
                'market': dump['market'],
                'asks': [[float(ask[0]), float(ask[1])] for ask in dump['asks']],
                'bids': [[float(bid[0]), float(bid[1])] for bid in dump['bids']]
            }
        )
        
        
    def __parse_snapshot_lvl3(self, dump):
        orders = dump['asks'] + dump['bids']
        for i in orders:
            i['market'] = dump['market']
            i['type'] = 'open'
            
        self.__message_event(MessageType.Level3.Snapshot, 
            orders
        )
    
    
    def __parse_lv3(self, dump):
        if dump['type'] == 'done':
            return {
                'market': dump['market'],
                'type': dump['type'],
                'orderId': dump['orderId'],
                'clientId': dump['clientId'],
                'side': dump['side'],
                'account': dump['account'],
                'accountSlot': dump['accountSlot'],
                'reason': dump['reason']
            }
        else:
            return {
                'market': dump['market'],
                'type': dump['type'],
                'orderId': dump['orderId'],
                'clientId': dump['clientId'],
                'side': dump['side'],
                'price': dump['price'],
                'size': dump['size'],
                'account': dump['account'],
                'accountSlot': dump['accountSlot']
            }
        
        
    def __parse_open_lv3(self, dump):        
        self.__message_event(MessageType.Level3.Open, 
            self.__parse_lv3(dump)
        )
        
        
    def __parse_change_lv3(self, dump):
        self.__message_event(MessageType.Level3.Change, 
            self.__parse_lv3(dump)
        )
        
        
    def __parse_fill_lv3(self, dump):
        self.__message_event(MessageType.Level3.Fill, 
            self.__parse_lv3(dump)
        )
        
        
    def __parse_done_lv3(self, dump):
        self.__message_event(MessageType.Level3.Fill, 
            self.__parse_lv3(dump)
        )