import websocket
from models import *
import json
import threading
from decimal import *
from sortedcontainers import SortedList    


WSS_URL = 'wss://api.serum-vial.dev/v1/ws'

class Wrapper:
    def __init__(self, logger, message_event, information_event):
        self.__message_event = message_event
        self.__information_event = information_event
        self.__logger = logger
        self.__channel_list = []
        self.__snapshots_map = {}
        self.__ws = SocketObj(WSS_URL, self.__logger, self.__m_event, self.__i_event)
        
        
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
                
                
    def __i_event(self, event, msg = None):
        match event:
            case MessageType.Event.Subscribed:
                self.__channel_list.append(msg)
            case MessageType.Event.Unsubscribed:
                self.__channel_list.remove(msg)
        
        self.__information_event(event, msg)
        
    
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

        self.__logger.debug('connection started')

        self.__thread_warning = threading.Thread(name='WarningLoop', daemon=True)
        self.__thread_warning.start()


    def stop(self):
        self.__ws.close()
        self.__logger.debug('stopped')


    def isStarted(self):
        return self.__isStarted


    def send(self, msg):
        try:
            self.__ws.send(msg)
        except:
            pass
        
    def __on_error(self, ws, error):
        self.__logger.info(f'<-- Error {error}')
        self.__information_event(BrokerEvent.SessionError, error)


    def __on_open(self, ws):
        self.__information_event(BrokerEvent.SessionLogon)
        self.__logger.info('<-- connection open')
        self.__isConnected = True


    def __on_close(self, ws, close_status_code, close_msg):
        self.__isConnected = False
        self.__information_event(BrokerEvent.SessionLogout)
        self.__logger.info(f'<-- connection close  code--{close_status_code}  msg--{close_msg}')
        

    def __on_message(self, ws, message):
        dump = json.loads(message)
        
        match dump['type']:
            case MessageType.Level1.Quote:
                self.__parse_quote(dump)
            case MessageType.Level2.Snapshot:
                self.__parse_snapshot_lvl2(dump)
            case MessageType.Level2.Update:
                self.__parse_update_lvl2(dump)
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
                'bestAsk': [Decimal(dump['bestAsk'][0]), Decimal(dump['bestAsk'][1])],
                'bestBid': [Decimal(dump['bestBid'][0]), Decimal(dump['bestBid'][1])]
            }
        )
        
        
    def __parse_snapshot_lvl2(self, dump):
        self.__message_event(MessageType.Level2.Snapshot, 
            {
                'market': dump['market'],
                'asks': [[Decimal(ask[0]), Decimal(ask[1])] for ask in dump['asks']],
                'bids': [[Decimal(bid[0]), Decimal(bid[1])] for bid in dump['bids']]
            }
        )
    
    
    def __parse_update_lvl2(self, dump):
        self.__message_event(MessageType.Level2.Update, 
            {
                'market': dump['market'],
                'asks': [[Decimal(ask[0]), Decimal(ask[1])] for ask in dump['asks']],
                'bids': [[Decimal(bid[0]), Decimal(bid[1])] for bid in dump['bids']]
            }
        )