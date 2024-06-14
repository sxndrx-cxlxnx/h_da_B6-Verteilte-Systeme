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
        print("addBet called")
        file = open("betOptions.txt", "a")
        file.write(nameanbieter + ":" + str(betid) + ":" + str(gameid) + ":" + str(winachance) + ":" + str(winbchance) + ":" + str(even) + ";\n")
        file.close()
        return True

    def changeBet(self, nameanbieter, betid, gameid, winachance, winbchance, even):
        print("changeBet called")
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
        print("deleteBet called")
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
        print("getMyBetOptions called")

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
        print("getAllBetOptions called")

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
        print("getSpecificBetOption called")
        with open('betOptions.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)
        
        for line in data:
            if line.startswith(nameanbieter + ":" + str(id) + ":") :
                return line
        
        return "SpecificBetOption not found >>" + nameanbieter + str(id) + "<<"

    def getAllPlacedBets(self):
        print("getAllPlacedBets called")

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
        print("getAllPlacedBetsToABetOption called")

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
        print("placeNewBet called")

        file = open("placedBets.txt", "a")
        file.write(nameanbieter + ":" + str(betid) + ":" + namebettor + ":" + str(moneyset) + ":" + str(ending) + ":" + str(winachance) + ":" + str(winbchance) + ":" + str(even) + ";\n")
        file.close()
        return True

    def betExists(self, nameanbieter, betid):
        print("betExists called")
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
        print("addGame called")
        file = open("openGames.csv", "a")
        file.write(str(gameid) + ":" + playerA + ":" + playerB + ";\n")
        file.close()
        return True

    def endGame(self, gameid, pointsA, pointsB):
        print("endGame called")
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

        # get betids and bookholders for game and delete BetOfffer------------------
        with open('betOptions.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        
        arrayOfBookholdersAndBetids = []

        with open("betOptions.txt", "w", encoding='utf-8') as file:
            for line in data:
                shouldBeDeleted = False
                x = line.split(':')
                if(int(x[2]) == gameid):
                    shouldBeDeleted = True
                    earningBookholder = 0
                    # iterate all placed bets---------------------------------------
                    with open('placedBets.txt','r',encoding='utf-8') as file2:
                        data2 = file2.readlines()       
            
                    for line2 in data2:
                        x2 = line2.split(':')
                        if line2.startswith(x[0] + ":" + x[1] + ":") :
                            pEnding = int(x2[4])
                            if(pEnding == ending):     
                                indexQuote = 4 + pEnding
                                quote = x2[indexQuote]
                                if(indexQuote == 7):
                                    quote = quote.split(';')[0]
                                earningBookholder -= float(x2[3]) * float(quote)
                                print(earningBookholder)
                            else:               
                                earningBookholder += float(x2[3])
                                print(earningBookholder)             
                    # --------------------------------------------------------------
                    print(earningBookholder)
                    file2 = open("finishedBetsBookholder.txt", "a")
                    file2.write(x[0] + ':' + x[1] + ':' + str(gameid)  + ':' +  str(int(earningBookholder)) + ';\n')
                    file2.close()
                    arrayOfBookholdersAndBetids.append(x[0] + ':' + x[1] + ':')

                if not shouldBeDeleted:
                        file.write(line)

        # earnings Calculation------------------------------------------------------------------
        with open('placedBets.txt','r',encoding='utf-8') as file:
            data = file.readlines()       

        returnValue = 0
        moneyBettor = 0
        
        with open("placedBets.txt", "w", encoding='utf-8') as file:
            for line in data:
                shouldBeDeleted = False
                for betOption in arrayOfBookholdersAndBetids:
                    if line.startswith(betOption) :
                        shouldBeDeleted = True
                        x = line.split(':')
                        pEnding = int(x[4])
                        if(pEnding == ending): 

                            indexQuote = 4 + pEnding
                            quote = x2[indexQuote]
                            if(indexQuote == 7):
                                quote = quote.split(';')[0]

                            moneyBettor += float(x[3]) * float(quote)
                            file2 = open("finishedBetsBettor.txt", "a")
                            file2.write(x[2] + ':' + str(gameid) + ':' + x[0] + ':' + x[1] + ':' + str(int(moneyBettor)) + ';\n')
                            file2.close()                            
                        else:
                            moneyBettor = 0
                            file2 = open("finishedBetsBettor.txt", "a")
                            file2.write(x[2] + ':' + str(gameid) + ':' + x[0] + ':' + x[1] + ':' + str(int(moneyBettor)) + ';\n')
                            file2.close()

                if not shouldBeDeleted:
                    file.write(line)

        return True


    def isGameOpen(self, gameid):
        print("isGameOpen called")
        with open('openGames.csv','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)     

        for line in data:
            if line.startswith(str(gameid) + ":") :
                return True

        return False

    def getAllOpenGames(self):
        print("getAllOpenGames called")

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
        print("getAllClosedGames called")

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
        print("getSpecificGame called")

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

    def getPossibleEarningBookholder(self, nameanbieter, betid, ending):
        print("getPossibleEarningBookholder called")
        with open('placedBets.txt','r',encoding='utf-8') as file:
            data = file.readlines()       

        returnValue = 0
        
        for line in data:
            if line.startswith(nameanbieter + ":" + str(betid) + ":") :
                pEnding = int(x[4])
                if(pEnding == ending):
                    x = line.split(':')                
                    returnValue -= float(x[3]) * float(x[4 + pEnding])
                else:
                    x = line.split(':')                
                    returnValue += float(x[3])           
            
        return returnValue

    def getNumberOfBettors(self, nameanbieter, betid):
        print("getNumberOfBettors called")
        with open('placedBets.txt','r',encoding='utf-8') as file:
            data = file.readlines()       

        returnValue = 0
        
        for line in data:
            if line.startswith(nameanbieter + ":" + str(betid) + ":") :
                returnValue += 1

        return returnValue

    def getEarningBettor(self, nameanbieter, betid, namebettor):
        print("getEarningBettor called")

        with open('finishedBetsBettor.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)
        
        for line in data:
            if line.startswith(namebettor + ":") :
                x = line.split(':')
                if x[2] == nameanbieter and int(x[3]) == betid:
                    return int(x[-1].split(';')[0])
        
        return -1
    
    def getEarningBookholder(self, nameanbieter, betid):
        print("getEarningBookholder called")

        with open('finishedBetsBookholder.txt','r',encoding='utf-8') as file:
            data = file.readlines()
        print(data)
        
        for line in data:
            if line.startswith(nameanbieter + ':' + str(betid) + ':') :
                x = line.split(':')                
                return int(x[-1].split(';')[0])
        
        return -1



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