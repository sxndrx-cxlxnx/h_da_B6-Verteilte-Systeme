#!/usr/bin/env python

# This server demonstrates Thrift's connection and "oneway" asynchronous jobs
# showCurrentTimestamp : which returns current time stamp from server
# asynchronousJob() : prints something, waits 10 secs and print another string
#
# source: https://cloudxlab.com/blog/creating-thrift-service/

port = 9090

import sys
# your gen-py dir
sys.path.append('../gen-py')

import time

# Wettanbieter files
from Wettanbieter import *
from Wettanbieter.ttypes import *

# Thrift files
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.server import TServer

# Server implementation
class WettanbieterHandler:

    def addBet(self, nameanbieter, id, playera, playerb, winachance, winbchance, even):
        file = open("bets.txt", "a")
        file.write(nameanbieter + ":" + str(id) + ":" + playera + ":" + playerb + ":" + str(winachance) + ":" + str(winbchance) + ":" + str(even) + "\n")
        file.close()
        return True

    def changeBet(self, nameanbieter, id, playera, playerb, winachance, winbchance, even):
        # source: https://www.geeksforgeeks.org/python-program-to-replace-specific-line-in-file/
        # source: https://stackoverflow.com/questions/4710067/how-to-delete-a-specific-line-in-a-text-file-using-python
        with open('bets.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        with open("bets.txt", "w", encoding='utf-8') as file:
            for line in data:
                if line.startswith(nameanbieter + ":" + str(id) + ":") :
                    file.write(nameanbieter + ":" + str(id) + ":" + playera + ":" + playerb + ":" + str(winachance) + ":" + str(winbchance) + ":" + str(even) + "\n")
                else :
                    file.write(line)

        return True

    def deleteBet(self, nameanbieter, id):
        # source: https://www.geeksforgeeks.org/python-program-to-replace-specific-line-in-file/
        # source: https://stackoverflow.com/questions/4710067/how-to-delete-a-specific-line-in-a-text-file-using-python
        
        with open('bets.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        with open("bets.txt", "w", encoding='utf-8') as file:
            for line in data:
                if not line.startswith(nameanbieter + ":" + str(id) + ":") :
                    file.write(line)

        return True
    
    def getMyBets(self, nameanbieter):

        with open('bets.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        returnValue = ""
        
        for line in data:
            if line.startswith(nameanbieter + ":") :
                returnValue += line + "\n"
            
        return returnValue

    def getBets(self, nameanbieter, id):
        with open('bets.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        returnValue = ""
        
        for line in data:
            if line.startswith(nameanbieter + ":" + str(id) + ":") :
                returnValue += line
            
        return returnValue

    def getBettors(self, nameanbieter, id):

        with open('bettor.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        returnValue = ""
        
        for line in data:
            if line.startswith(nameanbieter + ":" + str(id) + ":") :
                returnValue += line
            
        return returnValue


    def setBettor(self, nameanbieter, betid, namebettor, moneyset, ending, winachance, winbchance, even):

        file = open("bettor.txt", "a")
        file.write(nameanbieter + ":" + str(betid) + ":" + namebettor + ":" + str(moneyset) + ":" + str(ending) + ":" + str(winachance) + ":" + str(winbchance) + ":" + str(even) + "\n")
        file.close()
        return True



# set handler to our implementation
handler = WettanbieterHandler()

processor = Wettanbieter.Processor(handler)
transport = TSocket.TServerSocket("0.0.0.0", port)
tfactory = TTransport.TBufferedTransportFactory()
pfactory = TBinaryProtocol.TBinaryProtocolFactory()

# set server
server = TServer.TThreadedServer(processor, transport, tfactory, pfactory)

print ('Starting server')
server.serve()