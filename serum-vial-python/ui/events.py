from serum_modules.models import *
COUNT = 5

class EventHandler:
    def __init__(self, logger):
        self.__logger = logger

    def message_event(self, event, msg):
        match event:
            case Channels.Level1: 
                self.__logger.info(f"{msg['market']}\n\tBid({msg['bestBid'][0]}) BidSize({msg['bestBid'][1]}) --- Ask({msg['bestAsk'][0]}) AskSize({msg['bestAsk'][1]})\n")
            case Channels.Level2:

                fmsg = ''
                fmsg = fmsg + f"{msg['market']}\n" + f"\tBids:\n"
                for i in msg['bids'][:COUNT]:
                    fmsg = fmsg + f'\tPrice({i[0]}) Amount({i[1]})\n'

                fmsg = fmsg + f"\n\tAsks:\n"
                for i in msg['asks'][:COUNT]:
                    fmsg = fmsg + f'\tPrice({i[0]}) Amount({i[1]})\n'

                self.__logger.info(fmsg)
            


    def information_event(self, event, msg = None):
        pass
        # match event:
        #     case MessageType.Event.Subscribed:
        #         self.__logger.info(f'Subscribed channel: {msg.channel} | market: {msg.market}')
        #     case MessageType.Event.Unsubscribed:
        #         self.__logger.info(f'Unsubscribed channel: {msg.channel} | market: {msg.market}')
        #     case _:
        #         self.__logger.info(f'{event} -- msg({msg if msg != None else ""})\n')