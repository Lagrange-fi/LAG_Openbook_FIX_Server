from wrapper import Wrapper
from models import *
from pprint import pprint
import datetime 
from config import KEYPAIR_PHANTOM

COUNT = 10

class Logger:
    def info(self, mess):
        now = datetime.datetime.now()
        print(f'{now.hour}:{now.minute}:{now.second} | <-- info -- {mess}')
        
    def error(self, mess):
        now = datetime.datetime.now()
        print(f'{now.hour}:{now.minute}:{now.second} | <-- error -- {mess}')
        
    def debug(self, mess):
        now = datetime.datetime.now()
        print(f'{now.hour}:{now.minute}:{now.second} | <-- debug -- {mess}')
        
        
def m_event(event, msg):
    now = datetime.datetime.now()
    match event:
        case Channels.Level1:
            print(f"{now.hour}:{now.minute}:{now.second} | <-- {msg['market']} BestAsk({msg['bestAsk']}) BestBid({msg['bestBid']})\n")
        case Channels.Level2:
            print(f"{now.hour}:{now.minute}:{now.second} | <-- {msg['market']}")
            print('Asks')
            pprint(msg['asks'][:COUNT]) 
            print()
            print('Bids')
            pprint(msg['bids'][:COUNT])
            print()


def i_event(event, msg = None):
    now = datetime.datetime.now()
    match event:
            case MessageType.Event.Subscribed:
                print(f'{now.hour}:{now.minute}:{now.second} | <-- Subscribe | channel: {msg.channel} | market: {msg.market}\n')
            case MessageType.Event.Unsubscribed:
                print(f'{now.hour}:{now.minute}:{now.second} | <-- Unsubscribe | channel: {msg.channel} | market: {msg.market}\n')
            case _:
                print(f'{now.hour}:{now.minute}:{now.second} | <-- {event} -- msg({msg if msg != None else ""})\n')

if __name__ == '__main__':
    wr = Wrapper(KEYPAIR_PHANTOM, Logger(), m_event, i_event)
    
    wr.start()
    
    btc_usdt = Instrument('BTC', 'USDT')
    eth_usdt = Instrument('ETH', 'USDT')
    
    sol_usdc = Instrument('SOL', 'USDC')
    
    order_buy = Order(sol_usdc.first, sol_usdc.second, 0.1, 82.34, Side.BUY)
    order_sell = Order(sol_usdc.first, sol_usdc.second, 0.1, 95.67, Side.SELL)
    
    while True:
        msg = input()
        match msg:
            case "l1":
                wr.subscribe(Channels.Level1, btc_usdt)
            case "ul1":
                wr.unsubscribe(Channels.Level1, btc_usdt)
            case "l2":
                wr.subscribe(Channels.Level2, eth_usdt)
            case "ul2":
                wr.unsubscribe(Channels.Level2, eth_usdt)
                
            case "sb":
                order_buy = wr.send_new_order(order_buy)
            case "cb": 
                wr.cancel_order(order_buy)
            case "ss":
                order_sell = wr.send_new_order(order_sell)
            case "cs":
                wr.cancel_order(order_sell)