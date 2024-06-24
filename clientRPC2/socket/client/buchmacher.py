import sys
import paho.mqtt.client as paho
import random
from threading import Thread
import time

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

# https://medium.com/@potekh.anastasia/a-beginners-guide-to-mqtt-understanding-mqtt-mosquitto-broker-and-paho-python-mqtt-client-990822274923

# Anbieter defaults
#anbieterName = "Hogwarts"
anbieterName = "Fred_und_George"
anbieterID = -1
wettIDctr = int(10)
quotenRangeStart = 100
quotenRangeStopp = 500
maxRisiko = -500
anbieterGebuehr = 6

leaderName = "null"
leaderID = 1000

variance = 2.1

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

Wettanbieter2ID = {}
# wettanbieterName : id

AnswersForGame = {}
# spielid : {
#   wettanbieterName : {
#       quoteA = 1.0
#       quoteB = 1.0
#       quoteEven = 1.0
#   }
#}

VotingForRecommendation = {}
# spielid : {
#   wettanbieterName : True/False
#}

Recommendations = {}
# spielid : {
#    quoteA = 1.0
#    quoteB = 1.0
#    quoteEven = 1.0
#}

RecommendationsSend = {}
#spielID = True/False


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

    publish_client = paho.Client()
    if publish_client.connect("mqttbroker", 1883, 60) != 0:
        print("Couldn't connect to the mqtt broker")
        sys.exit(1)

    mqtt_message = anbieterName + ":" + str(wettIDctr) + ":" + str(quoteA) + ":" + str(quoteB) + ":" + str(quoteEven) + ":"

    publish_client.publish("neuesWettangebot", mqtt_message, 0)
    publish_client.disconnect()

    wettIDctr += 1
  
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

        print(Gewinnwahrscheinlichkeit)
        print(Gewinnwahrscheinlichkeit[spielID])
        print(Gewinnwahrscheinlichkeit[spielID]["A"])
        print(Gewinnwahrscheinlichkeit[spielID]["A"]["Spielstand"])
        

        aGewinnwahrscheinlichkeit = Gewinnwahrscheinlichkeit[spielID]["A"]["Spielstand"] + Gewinnwahrscheinlichkeit[spielID]["A"]["Verletzung"] + Gewinnwahrscheinlichkeit[spielID]["A"]["Dobby"] + Gewinnwahrscheinlichkeit[spielID]["A"]["Freiwurf"] + Gewinnwahrscheinlichkeit[spielID]["A"]["Platzverweis"]
        bGewinnwahrscheinlichkeit = Gewinnwahrscheinlichkeit[spielID]["B"]["Spielstand"] + Gewinnwahrscheinlichkeit[spielID]["B"]["Verletzung"] + Gewinnwahrscheinlichkeit[spielID]["B"]["Dobby"] + Gewinnwahrscheinlichkeit[spielID]["B"]["Freiwurf"] + Gewinnwahrscheinlichkeit[spielID]["B"]["Platzverweis"]
        evenGewinnwahrscheinlichkeit = 0.0


        if aGewinnwahrscheinlichkeit > bGewinnwahrscheinlichkeit:
 
            if aGewinnwahrscheinlichkeit - bGewinnwahrscheinlichkeit < 0.3 :
                evenGewinnwahrscheinlichkeit = 1.0
        elif bGewinnwahrscheinlichkeit > aGewinnwahrscheinlichkeit:
            if bGewinnwahrscheinlichkeit - aGewinnwahrscheinlichkeit < 0.3 :
                evenGewinnwahrscheinlichkeit = 1.0
        else:
            evenGewinnwahrscheinlichkeit = 1.0

        isWinALikely = False
        isWinBLikely = False
        isEvenLikely = False

        if aGewinnwahrscheinlichkeit > bGewinnwahrscheinlichkeit and aGewinnwahrscheinlichkeit > evenGewinnwahrscheinlichkeit:
            isWinALikely = True
        elif bGewinnwahrscheinlichkeit > aGewinnwahrscheinlichkeit and bGewinnwahrscheinlichkeit > evenGewinnwahrscheinlichkeit:
            isWinBLikely = True
        elif evenGewinnwahrscheinlichkeit > aGewinnwahrscheinlichkeit and evenGewinnwahrscheinlichkeit > bGewinnwahrscheinlichkeit:
            isEvenLikely = True

        if isRiskAToHigh and isWinALikely:
            cooperateRequest(spielID, 1, risikoA + gebuehrenEinnahmen, risikoB + gebuehrenEinnahmen, risikoEven + gebuehrenEinnahmen)
        elif isRiskBToHigh and isWinBLikely:
            cooperateRequest(spielID, 2, risikoA + gebuehrenEinnahmen, risikoB + gebuehrenEinnahmen, risikoEven + gebuehrenEinnahmen)
        elif isRiskEvenToHigh and isEvenLikely:
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

                publish_client = paho.Client()
                if publish_client.connect("mqttbroker", 1883, 60) != 0:
                    print("Couldn't connect to the mqtt broker")
                    sys.exit(1)

                mqtt_message = anbieterName + ":" + str(wettID) + ":" + str(newQuoteA) + ":" + str(newQuoteB) + ":" + str(newQuoteEven) + ":"

                publish_client.publish("neuesWettangebot", mqtt_message, 0)
                publish_client.disconnect()

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

                publish_client = paho.Client()
                if publish_client.connect("mqttbroker", 1883, 60) != 0:
                    print("Couldn't connect to the mqtt broker")
                    sys.exit(1)

                mqtt_message = anbieterName + ":" + str(wettID) + ":" + str(newQuoteA) + ":" + str(newQuoteB) + ":" + str(newQuoteEven) + ":"

                publish_client.publish("neuesWettangebot", mqtt_message, 0)
                publish_client.disconnect()

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

                publish_client = paho.Client()
                if publish_client.connect("mqttbroker", 1883, 60) != 0:
                    print("Couldn't connect to the mqtt broker")
                    sys.exit(1)

                mqtt_message = anbieterName + ":" + str(wettID) + ":" + str(newQuoteA) + ":" + str(newQuoteB) + ":" + str(newQuoteEven) + ":"

                publish_client.publish("neuesWettangebot", mqtt_message, 0)
                publish_client.disconnect()

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
    if team == 1:
        print(Gewinnwahrscheinlichkeit)
        Gewinnwahrscheinlichkeit[spielID]["A"]["Verletzung"] -= 0.2
        Gewinnwahrscheinlichkeit[spielID]["B"]["Verletzung"] += 0.2
    else:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Verletzung"] += 0.2
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
    if anzahlTeamA > anzahlTeamB:
        diff = anzahlTeamA - anzahlTeamB

        Gewinnwahrscheinlichkeit[spielID]["A"]["Platzverweis"] += diff * 0.3
        Gewinnwahrscheinlichkeit[spielID]["B"]["Platzverweis"] -= diff * 0.3

    elif anzahlTeamA < anzahlTeamB:
        diff = anzahlTeamB - anzahlTeamA

        Gewinnwahrscheinlichkeit[spielID]["A"]["Platzverweis"] -= diff * 0.3
        Gewinnwahrscheinlichkeit[spielID]["B"]["Platzverweis"] += diff * 0.3

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


    sendOffer = False

    if (IsendCooperationRequest[spielID] == False) and (ImInCooperation[spielID] == False):

        for andererAnbieter, obj in CooperationRequests[spielID].items():
            if not andererAnbieter == anbieterName:

                iSendCooperationOffer = False
                for x, obj in IsendCooperationOffer[spielID].items():
                    if x == andererAnbieter:
                        if IsendCooperationOffer[spielID][andererAnbieter]["OfferSend"] == False:
                            iSendCooperationOffer = True

                if not iSendCooperationOffer:
                    andererAnbieterTeam = CooperationRequests[spielID][andererAnbieter]["team"]

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
                            makeOfferToOther(andererAnbieter, spielID)
                            sendOffer = True
                
    
    if (IsendCooperationRequest[spielID] == False) and (sendOffer == False) and (ImInCooperation[spielID] == False):

        clientSend = paho.Client()

        if clientSend.connect("mqttbroker", 1883, 60) != 0:
            print("Couldn't connect to the mqtt broker")
            sys.exit(1)

        clientSend.publish("risikoKommunikation", message, 0)
        clientSend.disconnect()

        IsendCooperationRequest[spielID] = True

        CooperationRequests[spielID][anbieterName] = {}

        CooperationRequests[spielID][anbieterName]["team"] = team
        CooperationRequests[spielID][anbieterName]["risikoA"] = risikoA
        CooperationRequests[spielID][anbieterName]["risikoB"] = risikoB
        CooperationRequests[spielID][anbieterName]["risikoEven"] = risikoEven
        CooperationRequests[spielID][anbieterName]["inCooperation"] = False

def makeOfferToOther(andererAnbieter, spielID):
    print("makeOfferToOther called")
    global CooperationOffersIsend
    global IsendCooperationOffer


    message = str(spielID) + ':' + anbieterName + ':'

    CooperationOffersIsend[spielID][andererAnbieter] = {}
    CooperationOffersIsend[spielID][andererAnbieter]["Accepted"] = False
    CooperationOffersIsend[spielID][andererAnbieter]["Rejected"] = False

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

    message = "NACK" + ':' + anbieterName  + ':' + str(spielID) + ':'

    if ImInCooperation[spielID] == False:
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

    #CooperationRequests[spielID][anbieterName]["inCooperation"] = andererAnbieter
    ImInCooperation[spielID] = True
    IsendCooperationRequest["Accepted"] = True

    CooperationOffersIsend[spielID][andererAnbieter]["Accepted"] = True
    CooperationOffersIsend[spielID][andererAnbieter]["Rejected"] = False

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


    if not andererAnbieter == anbieterName:

        CooperationRequests[spielID][andererAnbieter] = {}

        CooperationRequests[spielID][andererAnbieter]["team"] = team
        CooperationRequests[spielID][andererAnbieter]["risikoA"] = risikoA
        CooperationRequests[spielID][andererAnbieter]["risikoB"] = risikoB
        CooperationRequests[spielID][andererAnbieter]["risikoEven"] = risikoEven
        CooperationRequests[spielID][andererAnbieter]["inCooperation"] = ""

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
    

    if team == 1 and ((otherRisikoA + risikoA) >= 0):
        makeOfferToOther(andererAnbieter, spielID)
    elif team == 2 and ((otherRisikoB + risikoB) >= 0):
        makeOfferToOther(andererAnbieter, spielID)
    elif team == 3 and ((otherRisikoEven + risikoEven) >= 0):
        makeOfferToOther(andererAnbieter, spielID)
    
def newPlacedBet(wettID, nameBettor, money, ending, quoteA, quoteB, quoteEven):
    print("newPlacedBet() called")
    try:
        transport = TSocket.TSocket( host , port)
        transport = TTransport.TBufferedTransport(transport)
        protocol = TBinaryProtocol.TBinaryProtocol(transport)

        clientrpc = Wettanbieter.Client(protocol)
        transport.open()
        clientrpc.placeNewBet(str(anbieterName), int(wettID),str(nameBettor), float(money), int(ending), float(quoteA), float(quoteB), float(quoteEven))
        transport.close()
        print("placed new bet from: ", nameBettor)

    except Thrift.TException as tx:
        print ('Something went wrong : %s' % (tx.message))

def handleNewDay():
    print("handleNewDay() called")
    global SpielIsStarted
    global anbieterName
    global leaderName
    global AnswersForGame
    global RecommendationsSend

    if anbieterName == leaderName:

        for spielID in SpielIsStarted:
            if SpielIsStarted[spielID] == False:
                AnswersForGame[spielID] = {}
                RecommendationsSend[spielID] = False
                print("Send Request for game ", spielID)

                message = str(spielID) + ':'
                clientSend = paho.Client()

                if clientSend.connect("mqttbroker", 1883, 60) != 0:
                    print("Couldn't connect to the mqtt broker")
                    sys.exit(1)

                clientSend.publish("anfrageEmpfehlungen", message, 0)
                clientSend.disconnect()

def handleRequestForGame(gameID):
    print("handleRequestForGame() called")
    global SpielID2WettID
    global anbieterName

    print(SpielID2WettID)
    quoteA = 0.0
    quoteB = 0.0
    quoteEven = 0.0

    wettID = SpielID2WettID.get(gameID)

    print("wettID", wettID)

    try:
        transport = TSocket.TSocket( host , port)
        transport = TTransport.TBufferedTransport(transport)
        protocol = TBinaryProtocol.TBinaryProtocol(transport)

        clientrpc = Wettanbieter.Client(protocol)
        transport.open()
        stringOption = clientrpc.getSpecificBetOption(anbieterName, wettID)
        print("stringOption ", stringOption)
        quoteA = float(stringOption.split(":")[3])
        quoteB = float(stringOption.split(":")[4])
        quoteEven = float(stringOption.split(":")[5].split(";")[0])
        
        transport.close()

    except Thrift.TException as tx:
        print ('Something went wrong : %s' % (tx.message))
    
    message = str(gameID) + ':' + anbieterName + ':' + str(quoteA) + ':' + str(quoteB) + ':' + str(quoteEven) + ':'

    clientSend = paho.Client()

    if clientSend.connect("mqttbroker", 1883, 60) != 0:
        print("Couldn't connect to the mqtt broker")
        sys.exit(1)

    clientSend.publish("empfehlungen", message, 0)
    clientSend.disconnect()

def handleAnswersForGame(gameID, anbieterName, quoteA, quoteB, quoteEven):
    print("handleAnswersForGame() called")
    global AnswersForGame

    print("AnswersForGame before handleRequestsForGame: ", AnswersForGame)

    AnswersForGame[gameID][anbieterName] = {}
    AnswersForGame[gameID][anbieterName]["quoteA"] = quoteA
    AnswersForGame[gameID][anbieterName]["quoteB"] = quoteB
    AnswersForGame[gameID][anbieterName]["quoteEven"] = quoteEven

    print("AnswersForGame after handleRequestsForGame: ", AnswersForGame)
    checkIfAllAnswered(gameID)

def checkIfAllAnswered(gameID):
    print("checkIfAllAnswered() called")
    global AnswersForGame
    global Wettanbieter2ID

    allAnswered = True

    print(Wettanbieter2ID)
    print(AnswersForGame)

    for wettanbieter in Wettanbieter2ID:
        print(wettanbieter)
        if not wettanbieter in AnswersForGame[gameID]:
            allAnswered = False

    if allAnswered:
        calculateRecommendation(gameID)

def calculateRecommendation(gameID):
    print("calculateRecommendation() called")
    global AnswersForGame
    global VotingForRecommendation
    global Recommendations

    newQuoteA = newQuoteB = newQuoteEven = 0.0
    counter = 0

    for wettanbieter in AnswersForGame[gameID]:
        newQuoteA += AnswersForGame[gameID][wettanbieter]["quoteA"]
        newQuoteB += AnswersForGame[gameID][wettanbieter]["quoteB"]
        newQuoteEven += AnswersForGame[gameID][wettanbieter]["quoteEven"]
        counter += 1
    
    print("counter: ", counter)
    print(newQuoteA)
    print(newQuoteB)
    print(newQuoteEven)

    newQuoteA = newQuoteA/counter
    newQuoteB = newQuoteB/counter
    newQuoteEven = newQuoteEven/counter

    print(newQuoteA)
    print(newQuoteB)
    print(newQuoteEven)

    print("Recommendations before change ", Recommendations)
    Recommendations[gameID] = {}
    Recommendations[gameID]["quoteA"] = newQuoteA
    Recommendations[gameID]["quoteB"] = newQuoteB
    Recommendations[gameID]["quoteEven"] = newQuoteEven
    print("Recommendations after change ", Recommendations)

    message = str(gameID) + ':' + str(newQuoteA) + ':' + str(newQuoteB) + ':' + str(newQuoteEven) + ':'

    clientSend = paho.Client()

    if clientSend.connect("mqttbroker", 1883, 60) != 0:
        print("Couldn't connect to the mqtt broker")
        sys.exit(1)

    clientSend.publish("empfehlungenVote", message, 0)
    clientSend.disconnect()

    VotingForRecommendation[gameID] = {}

def votingRecommendation(gameID, quoteA, quoteB, quoteEven):
    print("votingRecommendation() called")
    global anbieterName
    global variance 

    print("anbieterName: ",anbieterName)

    ownQuoteA = 0.0
    ownQuoteB = 0.0
    ownQuoteEven = 0.0

    wettID = SpielID2WettID.get(gameID)

    print("wettID", wettID)

    try:
        transport = TSocket.TSocket( host , port)
        transport = TTransport.TBufferedTransport(transport)
        protocol = TBinaryProtocol.TBinaryProtocol(transport)

        clientrpc = Wettanbieter.Client(protocol)
        transport.open()
        stringOption = clientrpc.getSpecificBetOption(anbieterName, wettID)
        print("stringOption ", stringOption)
        ownQuoteA = float(stringOption.split(":")[3])
        ownQuoteB = float(stringOption.split(":")[4])
        ownQuoteEven = float(stringOption.split(":")[5].split(";")[0])
        
        transport.close()

    except Thrift.TException as tx:
        print ('Something went wrong : %s' % (tx.message))

    message = "ACK" + ':' + str(gameID) + ':' + anbieterName + ':'
    if not ((quoteA <= ownQuoteA and ((ownQuoteA - quoteA) <= variance)) or (quoteA >= ownQuoteA and ((quoteA - ownQuoteA) <= variance))) :
        
        if quoteA < ownQuoteA:
            message = "NACK" + ':' + str(gameID) + ':' + anbieterName + ':' + "1" + ':' + "1" + ':' #zu klein
        else:
            message = "NACK" + ':' + str(gameID) + ':' + anbieterName + ':' + "1" + ':' + "2" + ':' #zu groß

    elif not ((quoteB <= ownQuoteB and ((ownQuoteB - quoteB) <= variance)) or (quoteB >= ownQuoteB and ((quoteB - ownQuoteB) <= variance))) :

        if quoteB < ownQuoteB:
            message = "NACK" + ':' + str(gameID) + ':' + anbieterName + ':' + "2" + ':' + "1" + ':' #zu klein
        else:
            message = "NACK" + ':' + str(gameID) + ':' + anbieterName + ':' + "2" + ':' + "2" + ':' #zu groß
        
    elif not ((quoteEven <= ownQuoteEven and ((ownQuoteEven - quoteEven) <= variance)) or (quoteEven >= ownQuoteEven and ((quoteEven - ownQuoteEven) <= variance))) :

        if quoteA < ownQuoteA:
            message = "NACK" + ':' + str(gameID) + ':' + anbieterName + ':' + "3" + ':' + "1" + ':' #zu klein
        else:
            message = "NACK" + ':' + str(gameID) + ':' + anbieterName + ':' + "3" + ':' + "2" + ':' #zu groß

    clientSend = paho.Client()

    if clientSend.connect("mqttbroker", 1883, 60) != 0:
        print("Couldn't connect to the mqtt broker")
        sys.exit(1)

    clientSend.publish("voting", message, 0)
    clientSend.disconnect()

def handleACKvoting(gameID, pAnbieterName):
    print("handleACKvoting() called")
    global VotingForRecommendation

    print("before ACK VotingForRecommendation ", VotingForRecommendation)

    VotingForRecommendation[gameID][pAnbieterName] = True
    print("after ACK VotingForRecommendation ", VotingForRecommendation)
    checkIfAllVoted(gameID)

def handleVeto(gameID, pAnbieterName, whichQuote, problem):
    print("handleVeto() called")
    global Recommendations
    global VotingForRecommendation

    print("VotingForRecommendation before: ", VotingForRecommendation)
    VotingForRecommendation[gameID] = {}
    print("VotingForRecommendation after: ", VotingForRecommendation)

    print("Recommendations before change ", Recommendations)
    if whichQuote == 1:
        if problem == 1:
            Recommendations[gameID]["quoteA"] += 0.1
        else:
            Recommendations[gameID]["quoteA"] -= 0.1

    elif whichQuote == 2:
        if problem == 1:
            Recommendations[gameID]["quoteB"] += 0.1
        else:
            Recommendations[gameID]["quoteB"] -= 0.1

    elif whichQuote == 3:
        if problem == 1:
            Recommendations[gameID]["quoteEven"] += 0.1
        else:
            Recommendations[gameID]["quoteEven"] -= 0.1

    print("Recommendations after change ", Recommendations)

    message = str(gameID) + ':' + str(Recommendations[gameID]["quoteA"]) + ':' + str(Recommendations[gameID]["quoteB"]) + ':' + str(Recommendations[gameID]["quoteEven"]) + ':'

    clientSend = paho.Client()

    if clientSend.connect("mqttbroker", 1883, 60) != 0:
        print("Couldn't connect to the mqtt broker")
        sys.exit(1)

    clientSend.publish("empfehlungenVote", message, 0)
    clientSend.disconnect()

def checkIfAllVoted(gameID):
    print("checkIfAllVoted() called")
    global VotingForRecommendation
    global Wettanbieter2ID
    global RecommendationsSend

    if RecommendationsSend[gameID] == False:

        allVoted = True

        print(Wettanbieter2ID)
        print(AnswersForGame)

        for wettanbieter in Wettanbieter2ID:
            print(wettanbieter)
            if not wettanbieter in VotingForRecommendation[gameID]:
                allVoted = False
            else:
                if VotingForRecommendation[gameID][wettanbieter] == False:
                    allVoted = False

        if allVoted:
            print("all accepted")
            sendRecommendation(gameID)

def sendRecommendation(gameID):
    print("sendRecommendation() called")
    global Recommendations
    global RecommendationsSend

    if RecommendationsSend[gameID] == False:

        recquotea = float(Recommendations[gameID]["quoteA"])
        recquoteb = float(Recommendations[gameID]["quoteB"])
        recquoteeven = float(Recommendations[gameID]["quoteEven"])

        try:
            transport = TSocket.TSocket( host , port)
            transport = TTransport.TBufferedTransport(transport)
            protocol = TBinaryProtocol.TBinaryProtocol(transport)

            clientrpc = Wettanbieter.Client(protocol)
            transport.open()
            clientrpc.newRecommendation(gameID, recquotea, recquoteb, recquoteeven)
            
            transport.close()
            print("new Recommendation for ", gameID, " send")
            RecommendationsSend[gameID] = True

        except Thrift.TException as tx:
            print ('Something went wrong : %s' % (tx.message))

def informAboutID():
    print("informAboutID() called")
    global anbieterName
    global anbieterID

    message = anbieterName + ':' + str(anbieterID) + ':'
    clientSend = paho.Client()

    if clientSend.connect("mqttbroker", 1883, 60) != 0:
        print("Couldn't connect to the mqtt broker")
        sys.exit(1)

    clientSend.publish("idKommunikation", message, 0)
    clientSend.disconnect()

def generateID():
    print("generateID() called")
    global anbieterID
    global anbieterName

    anbieterID = int(random.randrange(0, 10))    

    Wettanbieter2ID[anbieterName] = int(anbieterID)

    informAboutID()

def gotNewID(otherName, otherID):
    print("gotNewID() called")
    global Wettanbieter2ID

    if not otherName == anbieterName:
        if not otherID == anbieterID:
            print("Wettanbieter2ID before gotNewID ", Wettanbieter2ID)
            if otherName in Wettanbieter2ID:                
                Wettanbieter2ID[otherName] = int(otherID)
            else:
                Wettanbieter2ID[otherName] = int(otherID)
                informAboutID()
        else:
            message = "NACK" + ':' + otherName + ':' + str(otherID) + ':' 

            clientSend = paho.Client()

            if clientSend.connect("mqttbroker", 1883, 60) != 0:
                print("Couldn't connect to the mqtt broker")
                sys.exit(1)

            clientSend.publish("idKommunikation", message, 0)
            clientSend.disconnect()
            informAboutID()

    print("Wettanbieter2ID after gotNewID ", Wettanbieter2ID)

def handleNACKidCommunication(otherName, otherID):
    print("handleNACKidCommunication() called")
    global leaderName
    global leaderID

    leaderName = "Null"
    leaderID = 1000

    if otherName == anbieterName:
        generateID()
        findLeader()

def findLeader():
    print("findLeader() called")
    global Wettanbieter2ID
    global leaderName
    global leaderID

    leaderChanged = False

    lowestID = leaderID
    name = leaderName
    for x in Wettanbieter2ID:        
        print(Wettanbieter2ID[x])
        if Wettanbieter2ID[x] < lowestID:
            leaderChanged = True
            lowestID = Wettanbieter2ID[x]
            name = x

    print("lowest ID found: ", lowestID, ' ', name)

    if leaderChanged:
        print("leader changed")
        message = name + ':' + str(lowestID) + ':'
        leaderName = name
        leaderID = lowestID

        clientSend = paho.Client()

        if clientSend.connect("mqttbroker", 1883, 60) != 0:
            print("Couldn't connect to the mqtt broker")
            sys.exit(1)

        clientSend.publish("leiterKommunikation", message, 0)
        clientSend.disconnect()

def handleLeaderCommunication(otherName, otherID):
    print("handleLeaderCommunication() called")
    global leaderName
    global leaderID

    print("1")
    if not otherName == leaderName:
        if otherID < leaderID:
            leaderName = otherName
            leaderID = otherID
            print("leader changed", leaderName, leaderID)

        else:
            print("3")
            message = leaderName + ':' + str(leaderID) + ':'

            clientSend = paho.Client()

            if clientSend.connect("mqttbroker", 1883, 60) != 0:
                print("Couldn't connect to the mqtt broker")
                sys.exit(1)

            clientSend.publish("leiterKommunikation", message, 0)
            clientSend.disconnect()

            print("4")

def handleLastWill(otherName):
    print("handleLastWill() called")
    global Wettanbieter2ID
    global leaderName
    global leaderID

    print("1")

    Wettanbieter2ID.pop(otherName)

    print("2")
    print("Wettanbieter2ID", Wettanbieter2ID)

    if otherName == leaderName:
        print("3")
        leaderName = "Null"
        leaderID = 1000
        print("4")
        findLeader()

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Connection Lost. Sending Last Will...")
        client.publish("lastWill", anbieterName + ':', qos=1, retain=False)

def message_handling(client, userdata, msg):
    print(f"{msg.topic}: {msg.payload.decode()}")
    global SpielIsStarted
    global ImInCooperation
    global anbieterName
    global leaderName

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
            # anbieterName +  ":" + str(wettID) + ':' + "automatischeWette" + ":" + str(money) + ":" + str(ending)  + ':' + str(quoteA) + ':' + str(quoteB) + ':' + str(quoteEven) + ':'
            
            message = msg.payload.decode()
            adressat = str(message.split(":")[0])
            if adressat == anbieterName:
                wettID = int( msg.payload.decode().split(":")[1] )
                nameBettor = msg.payload.decode().split(":")[2]
                money = float( msg.payload.decode().split(":")[3] )
                ending = int( msg.payload.decode().split(":")[4] )
                quoteA = float( msg.payload.decode().split(":")[5] )
                quoteB = float( msg.payload.decode().split(":")[6] )
                quoteEven = float( msg.payload.decode().split(":")[7] )
                # TODO WettID - fuer platzierte Wette

                newPlacedBet(wettID, nameBettor, money, ending, quoteA, quoteB, quoteEven)
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

        case "newDay":
            print("new Day")
            handleNewDay()
            

        case "idKommunikation":
            print("idKommunikation")

            if not (msg.payload.decode().split(":")[0] == "NACK"):
                print("new ID Message")
                otherName = msg.payload.decode().split(":")[0]
                otherID = msg.payload.decode().split(":")[1]

                gotNewID(otherName, int(otherID))
            else:
                otherName = msg.payload.decode().split(":")[1]
                otherID = msg.payload.decode().split(":")[2]
                handleNACKidCommunication(otherName, int(otherID))

        case "leiterKommunikation":
            print("leiterKommunikation")
            otherName = msg.payload.decode().split(":")[0]
            otherID = msg.payload.decode().split(":")[1]
            handleLeaderCommunication(otherName, int(otherID))

        case "lastWill":
            print("lastWill")
            otherName = msg.payload.decode().split(":")[0]
            handleLastWill(otherName)

        case "anfrageEmpfehlungen":
            print("anfrageEmpfehlungen")
            gameID = msg.payload.decode().split(":")[0]
            handleRequestForGame(int(gameID))

        case "empfehlungen":
            print("empfehlungen")
            if leaderName == anbieterName:
                # gameID + ':' + anbieterName + ':' + str(quoteA) + ':' + str(quoteB) + ':' + str(quoteEven) + ':'
                gameID = msg.payload.decode().split(":")[0]
                pAnbieterName = msg.payload.decode().split(":")[1]
                quoteA = msg.payload.decode().split(":")[2]
                quoteB = msg.payload.decode().split(":")[3]
                quoteEven = msg.payload.decode().split(":")[4]
                handleAnswersForGame(int(gameID), pAnbieterName, float(quoteA), float(quoteB), float(quoteEven))

        case "empfehlungenVote":
            print("empfehlungenVote")
            #str(gameID) + ':' + str(newQuoteA) + ':' + str(newQuoteB) + ':' + str(newQuoteEven) + ':'
            gameID = msg.payload.decode().split(":")[0]
            quoteA = msg.payload.decode().split(":")[1]
            quoteB = msg.payload.decode().split(":")[2]
            quoteEven = msg.payload.decode().split(":")[3]
            votingRecommendation(int(gameID), float(quoteA), float(quoteB), float(quoteEven))

        case "voting":
            print("voting")
            if leaderName == anbieterName:
                #"ACK" + ':' + str(gameID) + ':' + anbieterName + ':'
                #"NACK" + ':' + str(gameID) + ':' + anbieterName + ':' + "1" + ':' + "2" + ':' #zu groß
                if msg.payload.decode().split(":")[0] == "ACK":
                    gameID = msg.payload.decode().split(":")[1]
                    pAnbieterName = msg.payload.decode().split(":")[2]
                    handleACKvoting(int(gameID), pAnbieterName)
                else:
                    gameID = msg.payload.decode().split(":")[1]
                    pAnbieterName = msg.payload.decode().split(":")[2]
                    whichQuote = msg.payload.decode().split(":")[3]
                    problem = msg.payload.decode().split(":")[4]
                    handleVeto(int(gameID), pAnbieterName, int(whichQuote), int(problem))

        case anbieterName:
            print("Personal Message")
            if msg.payload.decode().split(":")[0] == "NACK":
                # "NACK" + ':' + anbieterName  + ':' + str(spielID) + ':'
                andererAnbieter  = msg.payload.decode().split(":")[1]
                spielID = int(msg.payload.decode().split(":")[2])
                offerGotRejected(andererAnbieter, spielID)
            elif msg.payload.decode().split(":")[0] == "ACK":
                andererAnbieter  = msg.payload.decode().split(":")[1]
                spielID = int(msg.payload.decode().split(":")[2])
                offerGotAccepted(andererAnbieter, spielID)
            else:
                # spielID + ':' + anbieterName + ':'                
                spielID = int(msg.payload.decode().split(":")[0])
                andererAnbieter  = msg.payload.decode().split(":")[1]
                gotOfferFromOther(andererAnbieter, spielID)
       
def start():
    time.sleep(2)
    generateID()
    findLeader()

client = paho.Client()

client.will_set("lastWill", anbieterName + ':', qos=1, retain=False)

client.on_message = message_handling
#client.on_disconnect = on_disconnect

if client.connect("mqttbroker", 1883, 60) != 0:
    print("Couldn't connect to the mqtt broker")
    sys.exit(1)

client.subscribe("lastWill")

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

client.subscribe("newDay")
client.subscribe("idKommunikation")
client.subscribe("leiterKommunikation")
client.subscribe("anfrageEmpfehlungen")
client.subscribe("empfehlungen")
client.subscribe("empfehlungenVote")
client.subscribe("voting")



client.subscribe(anbieterName)


thread = Thread(target = start)
thread.start()

try:
    client.loop_forever()
except Exception:
    print("Caught an Exception, something went wrong...")
finally:
    print("Disconnecting from the MQTT broker")
    #client.disconnect()