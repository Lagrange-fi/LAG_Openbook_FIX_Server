from pyserum.enums import Side

class BrokerEvent:
    SessionLogon = "SessionLogon"
    SessionLogout = "SessionLogout"
    SessionError = "SessionError"


class Ops:
    Subscribe = 'subscribe'
    Unsubscribe = 'unsubscribe'
    
    
class Channels:
    Level1 = 'level1'
    Level2 = 'level2'
    Level3 = 'level3'
   
class MessageType: 
    class Event:
        Error = 'error'
        Subscribed = 'subscribed'
        Unsubscribed = 'unsubscribed'
        
    class Level1:
        Quote = 'quote'
        
    class Level2:
        Snapshot = 'l2snapshot'
        Update = 'l2update'
        
    class Level3:
        Snapshot = 'l3snapshot'
        Open = 'open'
        Change = 'change'
        Fill = 'fill'
        Done = 'done'

    
class Instrument:
    def __init__(self, first: str, second: str):
        self.__first = first.upper()
        self.__second = second.upper()
        
        
    @property
    def first(self):
        return self.__first
    
    
    @property
    def second(self):
        return self.__second
        
        
    @property
    def market(self):
        return f'{self.__first}/{self.__second}'
    
    
class SubscribedChannel:
    def __init__(self, channel: Channels, market: str):
        self.__channel = channel
        self.__market = market
        
        
    def __eq__(self, other):
        return  self.__channel == other.__channel and \
                self.__market ==  other.__market
                
    
    @property
    def channel(self):
        return self.__channel
    
    
    @property
    def market(self):
        return self.__market
    
class Order:
    def __init__(
        self, 
        first: str, 
        second: str, 
        amount: float, 
        price: float, 
        side: Side, 
        client_id: int = 0
    ):
        self.__first = first
        self.__second = second
        self.__client_id = client_id
        self.__amount = amount
        self.__price = price
        self.__side = side
        
    def __eq__(self, other) -> bool:
        return self.__client_id == other.__client_id
    
    @property
    def client_id(self) -> int:
        return self.__client_id
    
    @client_id.setter
    def client_id(self, new_id):
        if not isinstance(new_id, int):
            raise TypeError
        self.__client_id = new_id
    
    @property
    def amount(self) -> float:
        return self.__amount
    
    @property
    def price(self) -> float:
        return self.__price
    
    @property
    def side(self) -> Side:
        return self.__side
    
    @property
    def first(self) -> str:
        return self.__first
    
    @property
    def second(self) -> str:
        return self.__second
    
    @property
    def market(self) -> str:
        return f'{self.__first}/{self.__second}'