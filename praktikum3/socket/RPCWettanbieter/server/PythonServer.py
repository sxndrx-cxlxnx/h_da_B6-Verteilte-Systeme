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

    def addBet(self, nameanbieter, betid, gameid, winachance, winbchance, even):
        file = open("betOptions.txt", "a")
        file.write(nameanbieter + ":" + str(betid) + ":" + str(gameid) + ":" + str(winachance) + ":" + str(winbchance) + ":" + str(even) + ";\n")
        file.close()
        return True

    def changeBet(self, nameanbieter, betid, gameid, winachance, winbchance, even):
        # source: https://www.geeksforgeeks.org/python-program-to-replace-specific-line-in-file/
        # source: https://stackoverflow.com/questions/4710067/how-to-delete-a-specific-line-in-a-text-file-using-python
        with open('betOptions.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        with open("betOptions.txt", "w", encoding='utf-8') as file:
            for line in data:
                if line.startswith(nameanbieter + ":" + str(betid) + ":") :
                    file.write(nameanbieter + ":" + str(betid) + ":" + str(gameid) + ":" + str(winachance) + ":" + str(winbchance) + ":" + str(even) + ";\n")
                else :
                    file.write(line)

        return True

    def deleteBet(self, nameanbieter, id):
        # source: https://www.geeksforgeeks.org/python-program-to-replace-specific-line-in-file/
        # source: https://stackoverflow.com/questions/4710067/how-to-delete-a-specific-line-in-a-text-file-using-python
        
        with open('betOptions.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        with open("betOptions.txt", "w", encoding='utf-8') as file:
            for line in data:
                if not line.startswith(nameanbieter + ":" + str(id) + ":") :
                    file.write(line)

        return True
    
    def getMyBetOptions(self, nameanbieter):

        with open('betOptions.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        returnValue = ""
        ctr = 0
        
        for line in data:
            if line.startswith(nameanbieter + ":") :
                ctr += 1
                returnValue += line

        returnValue = str(ctr) + "#" + returnValue
            
        return returnValue

    def getAllBetOptions(self):

        with open('betOptions.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        returnValue = ""
        ctr = 0
        
        for line in data:
                ctr += 1
                returnValue += line

        returnValue = str(ctr) + "#" + returnValue
            
        return returnValue

    def getSpecificBetOption(self, nameanbieter, id):
        with open('betOptions.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)
        
        for line in data:
            if line.startswith(nameanbieter + ":" + str(id) + ":") :
                return line
        
        return "SpecificBetOption not found >>" + nameanbieter + str(id) + "<<"

    def getAllPlacedBets(self):

        with open('placedBets.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        returnValue = ""
        ctr = 0
        
        for line in data:
            ctr += 1
            returnValue += line

        returnValue = str(ctr) + "#" + returnValue
            
        return returnValue

    def getAllPlacedBetsToABetOption(self, nameanbieter, id):

        with open('placedBets.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        returnValue = ""
        ctr = 0
        
        for line in data:
            if line.startswith(nameanbieter + ":" + str(id) + ":") :
                ctr += 1
                returnValue += line

        returnValue = str(ctr) + "#" + returnValue
            
        return returnValue

    def placeNewBet(self, nameanbieter, betid, namebettor, moneyset, ending, winachance, winbchance, even):

        file = open("placedBets.txt", "a")
        file.write(nameanbieter + ":" + str(betid) + ":" + namebettor + ":" + str(moneyset) + ":" + str(ending) + ":" + str(winachance) + ":" + str(winbchance) + ":" + str(even) + ";\n")
        file.close()
        return True

    def betExists(self, nameanbieter, betid):
        with open('betOptions.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        
        
        for line in data:
            if line.startswith(nameanbieter + ":" + str(betid) + ":") :
                print("Bet Found")
                return True
        print("Bet not Found")    
        return False

    def addGame(self, gameid, playerA, playerB):
        file = open("openGames.csv", "a")
        file.write(str(gameid) + ":" + playerA + ":" + playerB + ";\n")
        file.close()
        return True

    def endGame(self, gameid, pointsA, pointsB):
        with open('openGames.csv','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)  

        gameline = ""

        with open("openGames.csv", "w", encoding='utf-8') as file:
            for line in data:
                if not line.startswith(str(gameid) + ":") :
                    file.write(line)
                else:
                    gameline = line        

        ending = 0

        if pointsA > pointsB:
            ending = 1
        elif pointsB > pointsA:
            ending = 2
        else:
            ending = 3
            
        print(gameline)
        gameline = gameline.replace(";\n", ":" + str(ending) + ":" + str(pointsA) + ":" + str(pointsB) + ";\n")
        print(gameline)

        file = open("closedGames.csv", "a")
        file.write(gameline)
        file.close()

        return True


    def isGameOpen(self, gameid):
        with open('openGames.csv','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)     

        for line in data:
            if line.startswith(str(gameid) + ":") :
                return True

        return False

    def getAllOpenGames(self):

        with open('openGames.csv','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        returnValue = ""
        ctr = 0
        
        for line in data:
            ctr += 1
            returnValue += line

        returnValue = str(ctr) + "#" + returnValue
            
        return returnValue


    def getAllClosedGames(self):

        with open('closedGames.csv','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)        

        returnValue = ""
        ctr = 0
        
        for line in data:
            ctr += 1
            returnValue += line

        returnValue = str(ctr) + "#" + returnValue
            
        return returnValue

    
    def getSpecificGame(self, gameid):

        with open('openGames.csv','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)     

        for line in data:
            if line.startswith(str(gameid) + ":") :
                return "open:" + line


        with open('closedGames.csv','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)     

        for line in data:
            if line.startswith(str(gameid) + ":") :
                return "closed:" + line

        return "game not found"


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