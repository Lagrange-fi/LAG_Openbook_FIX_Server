
import sys
from PyQt5 import QtWidgets
from ui import design
from ui.logger import Logger
from ui.events import EventHandler
from serum_modules.wrapper import Wrapper
from serum_modules.config import KEYPAIR_PHANTOM
from serum_modules.models import *

class MainApp(QtWidgets.QMainWindow, design.Ui_MainWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self) 
        self.load_wrapper()
        self.connect()

    def load_wrapper(self):
        self.__logger = Logger(self.LoggerBox)
        self.__event_handler = EventHandler(self.__logger)
        self.__wr = Wrapper(
            KEYPAIR_PHANTOM,
            self.__logger,
            self.__event_handler.message_event,
            self.__event_handler.information_event
        )

        self.__wr.start()

    
    def connect(self):
        self.SubscribeTop.clicked.connect(self.subscribe_top_event)
        self.UnsubscribeTop.clicked.connect(self.unsubscribe_top_event)

        self.SubscribeDepth.clicked.connect(self.subscribe_depth_event)
        self.UnsubscribeDepth.clicked.connect(self.unsubscribe_depth_event)
        
        
    def subscribe_top_event(self):
        instr = self.__get_instrument_from_widget()
        if instr == None:
            return

        self.__wr.subscribe(
            Channels.Level1, 
            instr
        )

    def unsubscribe_top_event(self):
        instr = self.__get_instrument_from_widget()
        if instr == None:
            return

        self.__wr.unsubscribe(
            Channels.Level1, 
            instr
        )

    def subscribe_depth_event(self):
        instr = self.__get_instrument_from_widget()
        if instr == None:
            return

        self.__wr.subscribe(
            Channels.Level2, 
            instr
        )

    def unsubscribe_depth_event(self):
        instr = self.__get_instrument_from_widget()
        if instr == None:
            return

        self.__wr.unsubscribe(
            Channels.Level2, 
            instr
        )

    def __get_instrument_from_widget(self):
        if self.Base.text() == "" or self.Quote.text() == "":
            self.__logger.error("Input fields must not be empty")
            return None
        
        return Instrument(self.Base.text().strip().upper(), self.Quote.text().strip().upper())

def main():
    app = QtWidgets.QApplication(sys.argv)  
    window = MainApp() 
    window.show()  
    app.exec_()  

if __name__ == '__main__':  
    main()