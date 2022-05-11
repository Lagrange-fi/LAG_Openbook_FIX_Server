import datetime 
from PyQt5.QtWidgets import QPlainTextEdit

class Logger:
    def __init__(self, output_device: QPlainTextEdit) -> None:
        self.__od = output_device
        self.__now = datetime.datetime.now
        
    
    def __log(self, msg):
        # self.__od.textCursor().movePosition(QTextCursor.end)
        self.__od.appendPlainText(msg)
        
    def info(self, msg):
        self.__log((f'{self.__now().hour}:{self.__now().minute}:{self.__now().second} | {msg}\n'))
        
    def error(self, msg):
        self.__log((f'{self.__now().hour}:{self.__now().minute}:{self.__now().second} | {msg}\n'))
        
    def debug(self, msg):
        pass
        # self.__log((f'{self.__now().hour}:{self.__now().minute}:{self.__now().second} | {msg}\n'))