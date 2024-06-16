import sys
import paho.mqtt.client as paho
import random

host = "p3-socket"
port = 9090

# your gen-py dir
sys.path.append('../gen-py')

# Wettanbieter files
from Wettanbieter import *
from Wettanbieter.ttypes import *

# Thrift files
from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

# Anbieter defaults
#anbieterName = "Hogwarts"
anbieterName = "Tagesprophet"
wettIDctr = int(10)
quotenRangeStart = 100
quotenRangeStopp = 500
maxRisiko = -500
anbieterGebuehr = 5

SpielID2WettID = {}
#"keySpielID" : "valueWettID"

WettID2SpielID = {}
#"keyWettID" : "valueSpielID"

SpielIsStarted = {}
#"keySpielID" : valueTrue
#"keySpielID" : valueFalse

Gewinnwahrscheinlichkeit = {}

#0 : {
#        "A" : {
#           "Spielstand"    : 1.2
#            "Verletzung"    : -0.2
#            "Dobby"         : +0.2
#            "Freiwurf"      : 0.0
#            "Platzverweis"  : 0.0
#        }
#        "A" : {
#            "Spielstand"    : 0.8
#           "Verletzung"    : 0.0
#            "Dobby"         : -0.2
#            "Freiwurf"      : 0.0
#            "Platzverweis"  : 0.0
#        }
#    }

CooperationRequests = {}
'''
spielid : {
        "WettanbieterName" : {
            "team"          : 1
            "risikoA"       : 0.0
            "risikoB"       : 0.0
            "risikoEven"    : 0.0
            "inCooperation" : andererAnbieter
        }
    }
'''

CooperationOffersIsend = {}
'''
spielid : {
        "WettanbieterName" : {
            "Accepted"   : False
            "Rejected"   : False
        }
    }
'''

IsendCooperationOffer = {}
'''
spielid : {
        "WettanbieterName" : {
            "OfferSend"   : False
        }
    }
'''

IsendCooperationRequest = {}
#"keySpielID" : valueTrue
#"keySpielID" : valueFalse

ImInCooperation = {}
#SpielID = True

# https://medium.com/@potekh.anastasia/a-beginners-guide-to-mqtt-understanding-mqtt-mosquitto-broker-and-paho-python-mqtt-client-990822274923

def newBetOption(spielID):
    print("newBetOption called")
    global wettIDctr
    global Gewinnwahrscheinlichkeit
    global CooperationRequests
    global CooperationOffersIsend
    global IsendCooperationOffer
    global SpielID2WettID
    global WettID2SpielID
    global SpielIsStarted

    quoteA = float(random.randrange(quotenRangeStart, quotenRangeStopp) / 100)
    quoteB = float(random.randrange(quotenRangeStart, quotenRangeStopp) / 100)
    quoteEven = float(random.randrange(quotenRangeStart, quotenRangeStopp) / 100)

    try:
        transport = TSocket.TSocket( host , port)
        transport = TTransport.TBufferedTransport(transport)
        protocol = TBinaryProtocol.TBinaryProtocol(transport)

        clientrpc = Wettanbieter.Client(protocol)
        transport.open()
        clientrpc.addBet(str(anbieterName), int(wettIDctr), int(spielID), quoteA, quoteB, quoteEven)
        transport.close()

    except Thrift.TException as tx:
        print ('Something went wrong : %s' % (tx.message))

    SpielID2WettID[spielID] = wettIDctr
    WettID2SpielID[wettIDctr] = spielID
    SpielIsStarted[spielID] = False
    wettIDctr += 1

    Gewinnwahrscheinlichkeit = {
        spielID : {
            "A" : {
                "Spielstand" : 1.0,
                "Verletzung": 0.0,
                "Dobby" : 0.0,
                "Freiwurf": 0.0,
                "Platzverweis": 0.0
            },
            "B" : {
                "Spielstand" : 1.0,
                "Verletzung": 0.0,
                "Dobby" : 0.0,
                "Freiwurf": 0.0,
                "Platzverweis": 0.0
            }
        }
    }

    CooperationRequests = {
        spielID : {}
    }

    CooperationOffersIsend = {    
        spielID : {}
    }
    IsendCooperationOffer = {
        spielID : {}
    }
    IsendCooperationRequest[spielID] = False
  

def calculateRisk(wettID, spielID):
    print("calculateRisk called")

    global Gewinnwahrscheinlichkeit
    global CooperationRequests
    global CooperationOffersIsend
    global IsendCooperationOffer
    global SpielID2WettID
    global WettID2SpielID
    global SpielIsStarted

    risikoA = risikoB = risikoEven = gebuehrenEinnahmen = 0
    try:
        transport = TSocket.TSocket( host , port)
        transport = TTransport.TBufferedTransport(transport)
        protocol = TBinaryProtocol.TBinaryProtocol(transport)

        clientrpc = Wettanbieter.Client(protocol)
        transport.open()
        risikoA = clientrpc.getPossibleEarningBookholder(anbieterName, int(wettID),1)
        risikoB = clientrpc.getPossibleEarningBookholder(anbieterName, int(wettID),2)
        risikoEven = clientrpc.getPossibleEarningBookholder(anbieterName, int(wettID),3)
        gebuehrenEinnahmen = clientrpc.getNumberOfBettors(anbieterName, int(wettID)) * anbieterGebuehr
        
        transport.close()
    except Thrift.TException as tx:
        print ('Something went wrong : %s' % (tx.message))

    isRiskAToHigh = risikoA + gebuehrenEinnahmen < maxRisiko
    isRiskBToHigh = risikoB + gebuehrenEinnahmen < maxRisiko
    isRiskEvenToHigh = risikoEven + gebuehrenEinnahmen < maxRisiko

    if SpielIsStarted[spielID]: # Buchmacherkommunikation
        print("SpielIsStarted - Hier wollen Buchmacher kooperieren")
        print("1")
        print(Gewinnwahrscheinlichkeit)
        print(Gewinnwahrscheinlichkeit[spielID])
        print(Gewinnwahrscheinlichkeit[spielID]["A"])
        print(Gewinnwahrscheinlichkeit[spielID]["A"]["Spielstand"])
        

        aGewinnwahrscheinlichkeit = Gewinnwahrscheinlichkeit[spielID]["A"]["Spielstand"] + Gewinnwahrscheinlichkeit[spielID]["A"]["Verletzung"] + Gewinnwahrscheinlichkeit[spielID]["A"]["Dobby"] + Gewinnwahrscheinlichkeit[spielID]["A"]["Freiwurf"] + Gewinnwahrscheinlichkeit[spielID]["A"]["Platzverweis"]
        print("2")
        bGewinnwahrscheinlichkeit = Gewinnwahrscheinlichkeit[spielID]["B"]["Spielstand"] + Gewinnwahrscheinlichkeit[spielID]["B"]["Verletzung"] + Gewinnwahrscheinlichkeit[spielID]["B"]["Dobby"] + Gewinnwahrscheinlichkeit[spielID]["B"]["Freiwurf"] + Gewinnwahrscheinlichkeit[spielID]["B"]["Platzverweis"]
        print("3")
        evenGewinnwahrscheinlichkeit = 0.0


        if aGewinnwahrscheinlichkeit > bGewinnwahrscheinlichkeit:
            print("4")
            if aGewinnwahrscheinlichkeit - bGewinnwahrscheinlichkeit < 0.3 :
                print("5")
                evenGewinnwahrscheinlichkeit = 1.0
        elif bGewinnwahrscheinlichkeit > aGewinnwahrscheinlichkeit:
            print("6")
            if bGewinnwahrscheinlichkeit - aGewinnwahrscheinlichkeit < 0.3 :
                print("7")
                evenGewinnwahrscheinlichkeit = 1.0
        else:
            print("8")
            evenGewinnwahrscheinlichkeit = 1.0

        isWinALikely = False
        isWinBLikely = False
        isEvenLikely = False

        if aGewinnwahrscheinlichkeit > bGewinnwahrscheinlichkeit and aGewinnwahrscheinlichkeit > evenGewinnwahrscheinlichkeit:
            print("9")
            isWinALikely = True
        elif bGewinnwahrscheinlichkeit > aGewinnwahrscheinlichkeit and bGewinnwahrscheinlichkeit > evenGewinnwahrscheinlichkeit:
            print("10")
            isWinBLikely = True
        elif evenGewinnwahrscheinlichkeit > aGewinnwahrscheinlichkeit and evenGewinnwahrscheinlichkeit > bGewinnwahrscheinlichkeit:
            print("11")
            isEvenLikely = True

        if isRiskAToHigh and isWinALikely:
            print("12")
            cooperateRequest(spielID, 1, risikoA + gebuehrenEinnahmen, risikoB + gebuehrenEinnahmen, risikoEven + gebuehrenEinnahmen)
        elif isRiskBToHigh and isWinBLikely:
            print("13")
            cooperateRequest(spielID, 2, risikoA + gebuehrenEinnahmen, risikoB + gebuehrenEinnahmen, risikoEven + gebuehrenEinnahmen)
        elif isRiskEvenToHigh and isEvenLikely:
            print("14")
            cooperateRequest(spielID, 3, risikoA + gebuehrenEinnahmen, risikoB + gebuehrenEinnahmen, risikoEven + gebuehrenEinnahmen)        


    else: # Quoten ändern

        if isRiskAToHigh:
            try:
                transport = TSocket.TSocket( host , port)
                transport = TTransport.TBufferedTransport(transport)
                protocol = TBinaryProtocol.TBinaryProtocol(transport)

                clientrpc = Wettanbieter.Client(protocol)
                transport.open()
                stringOption = clientrpc.getSpecificBetOption(anbieterName, wettID)
                quoteA = float(stringOption.split(":")[3])
                quoteB = float(stringOption.split(":")[4])
                quoteEven = float(stringOption.split(":")[5].split(";")[0])
                
                newQuoteA = quoteA * 0.8
                newQuoteB = quoteB
                newQuoteEven = quoteEven
                if newQuoteA < (quotenRangeStart / 100):
                    newQuoteA = (quotenRangeStart / 100)
                if not isRiskBToHigh:
                    newQuoteB = quoteB * 1.2
                    if newQuoteB > (quotenRangeStopp / 100):
                        newQuoteB = (quotenRangeStopp / 100)
                if not isRiskEvenToHigh:
                    newQuoteEven = quoteEven * 1.2
                    if newQuoteEven > (quotenRangeStopp / 100):
                        newQuoteEven = (quotenRangeStopp / 100)

                print("A) WettID:", wettID)
                clientrpc.changeBet(anbieterName, wettID, spielID, newQuoteA, newQuoteB, newQuoteEven)
                print("A) old Bet:", quoteA, quoteB, quoteEven)
                print("A) new Bet:", newQuoteA, newQuoteB, newQuoteEven)
                transport.close()

            except Thrift.TException as tx:
                print ('Something went wrong : %s' % (tx.message))

        if isRiskBToHigh:
            try:
                transport = TSocket.TSocket( host , port)
                transport = TTransport.TBufferedTransport(transport)
                protocol = TBinaryProtocol.TBinaryProtocol(transport)

                clientrpc = Wettanbieter.Client(protocol)
                transport.open()
                stringOption = clientrpc.getSpecificBetOption(anbieterName, wettID)
                quoteA = float(stringOption.split(":")[3])
                quoteB = float(stringOption.split(":")[4])
                quoteEven = float(stringOption.split(":")[5].split(";")[0])
                
                newQuoteB = quoteB * 0.8
                newQuoteA = quoteA
                newQuoteEven = quoteEven
                if newQuoteB < (quotenRangeStart / 100):
                    newQuoteB = (quotenRangeStart / 100)
                if not isRiskAToHigh:
                    newQuoteA = quoteA * 1.2
                    if newQuoteA > (quotenRangeStopp / 100):
                        newQuoteA = (quotenRangeStopp / 100)
                if not isRiskEvenToHigh:
                    newQuoteEven = quoteEven * 1.2
                    if newQuoteEven > (quotenRangeStopp / 100):
                        newQuoteEven = (quotenRangeStopp / 100)

                print("B) WettID:", wettID)
                clientrpc.changeBet(anbieterName, wettID, spielID, newQuoteA, newQuoteB, newQuoteEven)
                print("B) old Bet:", quoteA, quoteB, quoteEven)
                print("B) new Bet:", newQuoteA, newQuoteB, newQuoteEven)
                transport.close()

            except Thrift.TException as tx:
                print ('Something went wrong : %s' % (tx.message))

        if isRiskEvenToHigh:
            try:
                transport = TSocket.TSocket( host , port)
                transport = TTransport.TBufferedTransport(transport)
                protocol = TBinaryProtocol.TBinaryProtocol(transport)

                clientrpc = Wettanbieter.Client(protocol)
                transport.open()
                stringOption = clientrpc.getSpecificBetOption(anbieterName, wettID)
                quoteA = float(stringOption.split(":")[3])
                quoteB = float(stringOption.split(":")[4])
                quoteEven = float(stringOption.split(":")[5].split(";")[0])
                
                newQuoteA = quoteA
                newQuoteB = quoteB
                newQuoteEven = quoteEven * 0.8
                if newQuoteEven < (quotenRangeStart / 100):
                    newQuoteEven = (quotenRangeStart / 100)
                if not isRiskBToHigh:
                    newQuoteB = quoteB * 1.2
                    if newQuoteB > (quotenRangeStopp / 100):
                        newQuoteB = (quotenRangeStopp / 100)
                if not isRiskAToHigh:
                    newQuoteA = quoteA * 1.2
                    if newQuoteA > (quotenRangeStopp / 100):
                        newQuoteA = (quotenRangeStopp / 100)

                print("Even) WettID:", wettID)
                clientrpc.changeBet(anbieterName, wettID, spielID, newQuoteA, newQuoteB, newQuoteEven)
                print("Even) old Bet:", quoteA, quoteB, quoteEven)
                print("Even) new Bet:", newQuoteA, newQuoteB, newQuoteEven)
                transport.close()

            except Thrift.TException as tx:
                print ('Something went wrong : %s' % (tx.message))

def calculateRiskOfGame(spielID):
    global SpielID2WettID
    wettID = SpielID2WettID[spielID]
    calculateRisk(wettID, spielID)

def calculateRiskOfBet(wettID):
    global WettID2SpielID
    spielID = WettID2SpielID[wettID]
    calculateRisk(wettID, spielID)

def endGame(spielID):
    print("endGame called")
    global SpielID2WettID
    global WettID2SpielID
    global SpielIsStarted

    wettID = SpielID2WettID[spielID]
    earnings = 0

    try:
        transport = TSocket.TSocket( host , port)
        transport = TTransport.TBufferedTransport(transport)
        protocol = TBinaryProtocol.TBinaryProtocol(transport)

        clientrpc = Wettanbieter.Client(protocol)
        transport.open()
        earnings = clientrpc.getEarningBookholder(anbieterName, wettID)
        transport.close()

    except Thrift.TException as tx:
        print ('Something went wrong : %s' % (tx.message))

    print("WettID:", wettID, "Earnings:",earnings)
    SpielID2WettID.pop(spielID)
    WettID2SpielID.pop(wettID)
    SpielIsStarted.pop(spielID)

def spielStandUpdate(spielID, standA, standB):
    print("spielStandUpdate() called")
    """minDifferenz = 120
    maxDifferenz = 180
    # Schnatz fangen = 150 + Spielende"""

    global Gewinnwahrscheinlichkeit

    if standA > standB:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Spielstand"] = 1.2
        Gewinnwahrscheinlichkeit[spielID]["B"]["Spielstand"] = 0.8

        '''
        if standA - standB < maxDifferenz and standA - standB > minDifferenz:
            Gewinnwahrscheinlichkeit[spielID]["Even"]["Spielstand"] = 0.3
        else :
            Gewinnwahrscheinlichkeit[spielID]["Even"]["Spielstand"] = 0.0
        '''
    elif standB > standA:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Spielstand"] = 0.8
        Gewinnwahrscheinlichkeit[spielID]["B"]["Spielstand"] = 1.2

        '''
        if standB - standA < maxDifferenz and standB - standA > minDifferenz:
            Gewinnwahrscheinlichkeit[spielID]["Even"]["Spielstand"] = 0.3
        else :
            Gewinnwahrscheinlichkeit[spielID]["Even"]["Spielstand"] = 0.0
        '''
    '''
    else 
        Gewinnwahrscheinlichkeit[spielID]["Even"]["Spielstand"] = 0.0
    '''

def verletzung(spielID, team):
    print("verletzung() called")
    global Gewinnwahrscheinlichkeit
    print("1")
    if team == 1:
        print("2")
        print(Gewinnwahrscheinlichkeit)
        Gewinnwahrscheinlichkeit[spielID]["A"]["Verletzung"] -= 0.2
        print("3")
        Gewinnwahrscheinlichkeit[spielID]["B"]["Verletzung"] += 0.2
        print("4")
    else:
        print("5")
        Gewinnwahrscheinlichkeit[spielID]["A"]["Verletzung"] += 0.2
        print("6")
        Gewinnwahrscheinlichkeit[spielID]["B"]["Verletzung"] -= 0.2

def freiwurf(spielID, team):
    print("freiwurf() called")
    global Gewinnwahrscheinlichkeit
    if team == 1:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Freiwurf"] += 0.1
        Gewinnwahrscheinlichkeit[spielID]["B"]["Freiwurf"] -= 0.1
    else:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Freiwurf"] -= 0.1
        Gewinnwahrscheinlichkeit[spielID]["B"]["Freiwurf"] += 0.1

def platzverweis(spielID, anzahlTeamA, anzahlTeamB):
    print("platzverweis() called")
    global Gewinnwahrscheinlichkeit
    print("1")
    if anzahlTeamA > anzahlTeamB:
        print("2")
        diff = anzahlTeamA - anzahlTeamB
        print("3")
        print(Gewinnwahrscheinlichkeit)

        Gewinnwahrscheinlichkeit[spielID]["A"]["Platzverweis"] += diff * 0.3
        print("4")
        Gewinnwahrscheinlichkeit[spielID]["B"]["Platzverweis"] -= diff * 0.3

        print(Gewinnwahrscheinlichkeit)
    elif anzahlTeamA < anzahlTeamB:
        print("5")
        diff = anzahlTeamB - anzahlTeamA
        print("6")
        print(Gewinnwahrscheinlichkeit)

        Gewinnwahrscheinlichkeit[spielID]["A"]["Platzverweis"] -= diff * 0.3
        print("7")
        Gewinnwahrscheinlichkeit[spielID]["B"]["Platzverweis"] += diff * 0.3

        print(Gewinnwahrscheinlichkeit)
    else :
        Gewinnwahrscheinlichkeit[spielID]["A"]["Platzverweis"] = 0.0
        Gewinnwahrscheinlichkeit[spielID]["B"]["Platzverweis"] = 0.0

def dobby(spielID, team):
    print("dobby() called")
    global Gewinnwahrscheinlichkeit
    if team == 1:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Dobby"] -= 0.2
        Gewinnwahrscheinlichkeit[spielID]["B"]["Dobby"] += 0.2
    else:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Dobby"] += 0.2
        Gewinnwahrscheinlichkeit[spielID]["B"]["Dobby"] -= 0.2

def cooperateRequest(spielID, team, risikoA, risikoB, risikoEven):
    print("cooperateRequest called")

    global IsendCooperationRequest
    global ImInCooperation
    global CooperationRequests


    message = anbieterName + ':' + str(spielID) + ':' + str(team) + ':' + str(risikoA) + ':' + str(risikoB) + ':' + str(risikoEven) + ':'
    print(message)

    myRisk = risikoA

    if team == 2:
        myRisk = risikoB
    elif team == 3:
        myRisk = risikoEven

    print(str(myRisk))

    sendOffer = False

    print("1")
    print(IsendCooperationRequest)

    if (IsendCooperationRequest[spielID] == False) and (ImInCooperation[spielID] == False):
        print("2")
        print(CooperationRequests)

        for andererAnbieter, obj in CooperationRequests[spielID].items():
            print("3")
            if not andererAnbieter == anbieterName:
                print("4")

                iSendCooperationOffer = False
                for x, obj in IsendCooperationOffer[spielID].items():
                    print("5")
                    if x == andererAnbieter:
                        print("6")
                        if IsendCooperationOffer[spielID][andererAnbieter]["OfferSend"] == False:
                            print("7")
                            iSendCooperationOffer = True

                if not iSendCooperationOffer:
                    print("8")
                    andererAnbieterTeam = CooperationRequests[spielID][andererAnbieter]["team"]
                    print("9")

                    if not andererAnbieterTeam == team:
                        myRiskString = "risikoA"
                        if team == 2:
                            myRiskString = "risikoB"
                        elif team == 3:
                            myRiskString = "risikoEven"

                        otherRiskString = "risikoA"
                        if andererAnbieterTeam == 2:
                            otherRiskString = "risikoB"
                        elif andererAnbieterTeam == 3:
                            otherRiskString = "risikoEven"

                        if  ( ( CooperationRequests[spielID][andererAnbieter][myRiskString] + myRisk ) >= 0 ) and ( ( CooperationRequests[spielID][andererAnbieter][otherRiskString] + CooperationRequests[spielID][anbieterName][otherRiskString] ) >= 0 ):

                            print("10")
                            makeOfferToOther(andererAnbieter, spielID)
                            sendOffer = True
                
    
    print("11")
    if (IsendCooperationRequest[spielID] == False) and (sendOffer == False) and (ImInCooperation[spielID] == False):
        print("12")

        clientSend = paho.Client()

        if clientSend.connect("mqttbroker", 1883, 60) != 0:
            print("Couldn't connect to the mqtt broker")
            sys.exit(1)

        clientSend.publish("risikoKommunikation", message, 0)
        clientSend.disconnect()
        print("13")

        IsendCooperationRequest[spielID] = True
        print("14")

        print(CooperationRequests)
        CooperationRequests[spielID][anbieterName] = {}
        print(CooperationRequests)

        CooperationRequests[spielID][anbieterName]["team"] = team
        CooperationRequests[spielID][anbieterName]["risikoA"] = risikoA
        CooperationRequests[spielID][anbieterName]["risikoB"] = risikoB
        CooperationRequests[spielID][anbieterName]["risikoEven"] = risikoEven
        CooperationRequests[spielID][anbieterName]["inCooperation"] = False

        print(CooperationRequests)

        '''
        CooperationRequests[spielID] = {
            andererAnbieter : {
                "team"          : 1,
                "risikoA"       : 0.0,
                "risikoB"       : 0.0,
                "risikoEven"    : 0.0,
                "inCooperation" : ""
            }
        }
        '''
        print("15")

def makeOfferToOther(andererAnbieter, spielID):
    print("makeOfferToOther called")
    global CooperationOffersIsend
    global IsendCooperationOffer

    print("1")

    message = str(spielID) + ':' + anbieterName + ':'

    print(message)

    print(CooperationOffersIsend)

    CooperationOffersIsend[spielID][andererAnbieter] = {}
    CooperationOffersIsend[spielID][andererAnbieter]["Accepted"] = False
    CooperationOffersIsend[spielID][andererAnbieter]["Rejected"] = False

    print(CooperationOffersIsend)

    print(IsendCooperationOffer)

    IsendCooperationOffer[spielID][andererAnbieter]["OfferSend"] = True

    clientSend = paho.Client()

    if clientSend.connect("mqttbroker", 1883, 60) != 0:
        print("Couldn't connect to the mqtt broker")
        sys.exit(1)

    clientSend.publish(andererAnbieter, message, 0)
    clientSend.disconnect()


def gotOfferFromOther(andererAnbieter, spielID):
    print("gotOfferFromOther called")

    global CooperationRequests
    global ImInCooperation

    print("1")
    print(andererAnbieter)

    message = "NACK" + ':' + anbieterName  + ':' + str(spielID) + ':'
    print(ImInCooperation)
    print(ImInCooperation[spielID])

    if ImInCooperation[spielID] == False:
        print("2")
        message = "ACK" + ':' + anbieterName  + ':' + str(spielID) + ':'
        #CooperationRequests[spielID][anbieterName]["inCooperation"] = andererAnbieter
        ImInCooperation[spielID] = True
        print("I'm in Cooperation with ", andererAnbieter, " for game ", spielID)
    

    clientSend = paho.Client()

    if clientSend.connect("mqttbroker", 1883, 60) != 0:
        print("Couldn't connect to the mqtt broker")
        sys.exit(1)

    clientSend.publish(andererAnbieter, message, 0)
    clientSend.disconnect()
    

def offerGotAccepted(andererAnbieter, spielID):
    print("offerGotAccepted called")

    global CooperationRequests
    global ImInCooperation
    global IsendCooperationRequest
    global CooperationOffersIsend

    print("1")
    print(ImInCooperation)
    print(spielID)

    #CooperationRequests[spielID][anbieterName]["inCooperation"] = andererAnbieter
    ImInCooperation[spielID] = True
    print("2")
    IsendCooperationRequest["Accepted"] = True
    print("3")

    CooperationOffersIsend[spielID][andererAnbieter]["Accepted"] = True
    print("4")
    CooperationOffersIsend[spielID][andererAnbieter]["Rejected"] = False
    print("5")

    print("I'm in Cooperation with ", andererAnbieter, " for game ", spielID)

def offerGotRejected(andererAnbieter, spielID):
    print("offerGotRejected called")

    global CooperationOffersIsend

    CooperationOffersIsend[spielID][andererAnbieter]["Accepted"] = False
    CooperationOffersIsend[spielID][andererAnbieter]["Rejected"] = True


def getCooperationRequest(andererAnbieter ,spielID, team, risikoA, risikoB, risikoEven):
    print("getCooperationRequest called")

    global CooperationRequests
    global IsendCooperationOffer

    print("1")

    if not andererAnbieter == anbieterName:
        print("2")
        print(CooperationRequests)

        CooperationRequests[spielID][andererAnbieter] = {}
        print("3")

        print(CooperationRequests)

        CooperationRequests[spielID][andererAnbieter]["team"] = team
        CooperationRequests[spielID][andererAnbieter]["risikoA"] = risikoA
        CooperationRequests[spielID][andererAnbieter]["risikoB"] = risikoB
        CooperationRequests[spielID][andererAnbieter]["risikoEven"] = risikoEven
        CooperationRequests[spielID][andererAnbieter]["inCooperation"] = ""

        print(CooperationRequests)

        IsendCooperationOffer[spielID][andererAnbieter] = {}
        IsendCooperationOffer[spielID][andererAnbieter]["OfferSend"] = False
    
        evaluateCooperationRequest(andererAnbieter, spielID, team, risikoA, risikoB, risikoEven)


def evaluateCooperationRequest(andererAnbieter, spielID, team, otherRisikoA, otherRisikoB, otherRisikoEven):
    print("evaluateCooperationRequest() called")

    wettID = SpielID2WettID[spielID]

    risikoA = risikoB = risikoEven = gebuehrenEinnahmen = 0
    try:
        transport = TSocket.TSocket( host , port)
        transport = TTransport.TBufferedTransport(transport)
        protocol = TBinaryProtocol.TBinaryProtocol(transport)

        clientrpc = Wettanbieter.Client(protocol)
        transport.open()
        risikoA = clientrpc.getPossibleEarningBookholder(anbieterName, int(wettID),1)
        risikoB = clientrpc.getPossibleEarningBookholder(anbieterName, int(wettID),2)
        risikoEven = clientrpc.getPossibleEarningBookholder(anbieterName, int(wettID),3)
        gebuehrenEinnahmen = clientrpc.getNumberOfBettors(anbieterName, int(wettID)) * anbieterGebuehr
        
        transport.close()
    except Thrift.TException as tx:
        print ('Something went wrong : %s' % (tx.message))

    risikoA = risikoA + gebuehrenEinnahmen
    risikoB = risikoB + gebuehrenEinnahmen
    risikoEven = risikoEven + gebuehrenEinnahmen 
    
    print("1")
    print(otherRisikoA + risikoA)
    print(otherRisikoB + risikoB)
    print(otherRisikoEven + risikoEven)
    print(risikoA)
    print(risikoB)
    print(risikoEven)

    if team == 1 and ((otherRisikoA + risikoA) >= 0):
        print("2")
        makeOfferToOther(andererAnbieter, spielID)
    elif team == 2 and ((otherRisikoB + risikoB) >= 0):
        print("3")
        makeOfferToOther(andererAnbieter, spielID)
    elif team == 3 and ((otherRisikoEven + risikoEven) >= 0):
        print("4")
        makeOfferToOther(andererAnbieter, spielID)
    


def message_handling(client, userdata, msg):
    print(f"{msg.topic}: {msg.payload.decode()}")
    global SpielIsStarted
    global ImInCooperation

    match msg.topic:
        case "neuesSpiel":
            print("match case neues Spiel")
            spielID = int( msg.payload.decode().split(":")[0] )
            newBetOption(spielID)

        case "spielBeginn":
            print("match case spielBeginn")
            spielID = int( msg.payload.decode().split(":")[0] )
            SpielIsStarted[spielID] = True
            ImInCooperation[spielID] = False
            calculateRiskOfGame(spielID)

        case "neueAutowette":
            print("match case neue Autowette")
            wettID = int( msg.payload.decode().split(":")[0] )
            calculateRiskOfBet(wettID)

        case "spielEnde":
            print("match case spielEnde")
            spielID = int( msg.payload.decode().split(":")[0] )
            endGame(spielID)
        case "spielStandUpdate":
            spielID = int( msg.payload.decode().split(":")[0] )
            standA = int( msg.payload.decode().split(":")[1] )
            standB = int( msg.payload.decode().split(":")[2] )
            spielStandUpdate(spielID, standA, standB)
            calculateRiskOfGame(spielID)
        case "verletzung": # GW +-0.1
            spielID = int( msg.payload.decode().split(":")[0] )
            team = int( msg.payload.decode().split(":")[1] )
            verletzung(spielID, team)
            calculateRiskOfGame(spielID)
        case "freiwurf": # GW +-0.1
            spielID = int( msg.payload.decode().split(":")[0] )
            team = int( msg.payload.decode().split(":")[1] )
            freiwurf(spielID, team)
            calculateRiskOfGame(spielID)
        case "platzverweis": # GW +-1
            spielID = int( msg.payload.decode().split(":")[0] )
            anzahlTeamA = int( msg.payload.decode().split(":")[1] )
            anzahlTeamB = int( msg.payload.decode().split(":")[2] )
            platzverweis(spielID, anzahlTeamA, anzahlTeamB)
            calculateRiskOfGame(spielID)
        case "dobby":
            spielID = int( msg.payload.decode().split(":")[0] )
            team = int( msg.payload.decode().split(":")[1] )
            dobby(spielID, team)
            calculateRiskOfGame(spielID)
        case "risikoKommunikation":
            andererAnbieter = msg.payload.decode().split(":")[0]
            spielID = int( msg.payload.decode().split(":")[1] )
            team = int( msg.payload.decode().split(":")[2] )
            risikoA = float(msg.payload.decode().split(":")[3])
            risikoB = float(msg.payload.decode().split(":")[4])
            risikoEven = float(msg.payload.decode().split(":")[5])
            getCooperationRequest(andererAnbieter ,spielID, team, risikoA, risikoB, risikoEven)

        case anbieterName:
            print("Personal Message")
            if msg.payload.decode().split(":")[0] == "NACK":
                print("1")
                # "NACK" + ':' + anbieterName  + ':' + str(spielID) + ':'
                andererAnbieter  = msg.payload.decode().split(":")[1]
                spielID = int(msg.payload.decode().split(":")[2])
                offerGotRejected(andererAnbieter, spielID)
            elif msg.payload.decode().split(":")[0] == "ACK":
                print("2")
                andererAnbieter  = msg.payload.decode().split(":")[1]
                spielID = int(msg.payload.decode().split(":")[2])
                offerGotAccepted(andererAnbieter, spielID)
            else:
                print("3")
                # spielID + ':' + anbieterName + ':'                
                spielID = int(msg.payload.decode().split(":")[0])
                andererAnbieter  = msg.payload.decode().split(":")[1]
                gotOfferFromOther(andererAnbieter, spielID)



client = paho.Client()
client.on_message = message_handling

if client.connect("mqttbroker", 1883, 60) != 0:
    print("Couldn't connect to the mqtt broker")
    sys.exit(1)

# Autowetter
client.subscribe("neueAutowette")
# News
client.subscribe("neuesSpiel")
client.subscribe("spielBeginn")
client.subscribe("spielEnde")
client.subscribe("spielStandUpdate")
client.subscribe("verletzung") # GW +-0.1
client.subscribe("freiwurf") # GW +-0.1
client.subscribe("platzverweis") # GW +-1
client.subscribe("dobby") # GW 0.2
# Buchmacher
client.subscribe("risikoKommunikation")

client.subscribe(anbieterName)

try:
    print("Press CTRL+C to exit...")
    client.loop_forever()
except Exception:
    print("Caught an Exception, something went wrong...")
finally:
    print("Disconnecting from the MQTT broker")
    client.disconnect()