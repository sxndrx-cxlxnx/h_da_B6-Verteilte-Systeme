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
anbieterName = "Hogwarts"
wettIDctr = int(10)
quotenRangeStart = 100
quotenRangeStopp = 500
maxRisiko = -500
anbieterGebuehr = 5

SpielID2WettID = {
    #"keySpielID" : "valueWettID"
}

WettID2SpielID = {
    #"keyWettID" : "valueSpielID"
}

SpielIsStarted = {
    #"keySpielID" : valueTrue
    #"keySpielID" : valueFalse
}

Gewinnwahrscheinlichkeit{
    """
    0 : {
        "A" : {
            "Spielstand"    : 1.2
            "Verletzung"    : -0.2
            "Dobby"         : +0.2
            "Freiwurf"      : 0.0
            "Platzverweis"  : 0.0
        }
        "A" : {
            "Spielstand"    : 0.8
            "Verletzung"    : 0.0
            "Dobby"         : -0.2
            "Freiwurf"      : 0.0
            "Platzverweis"  : 0.0
        }
    }
    """
}

# https://medium.com/@potekh.anastasia/a-beginners-guide-to-mqtt-understanding-mqtt-mosquitto-broker-and-paho-python-mqtt-client-990822274923

def newBetOption(spielID):
    print("newBetOption called")
    global wettIDctr
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
    Gewinnwahrscheinlichkeit[spielID] = {
        "A" : {
            "Spielstand"    : 1.0
            "Verletzung"    : 0.0
            "Dobby"         : 0.0
            "Freiwurf"      : 0.0
            "Platzverweis"  : 0.0
        }
        "B" : {
            "Spielstand"    : 1.0
            "Verletzung"    : 0.0
            "Dobby"         : 0.0
            "Freiwurf"      : 0.0
            "Platzverweis"  : 0.0
        }
    }

def calculateRisk(wettID, spielID):
    print("calculateRisk called")
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

    if SpielIsStarted[spielID]: # Buchmacherkommunikation
        print("SpielIsStarted - Hier wollen Buchmacher wetten tauschen")
    else: # Quoten ändern
        isRiskAToHigh = risikoA + gebuehrenEinnahmen < maxRisiko
        isRiskBToHigh = risikoB + gebuehrenEinnahmen < maxRisiko
        isRiskEvenToHigh = risikoEven + gebuehrenEinnahmen < maxRisiko

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
    wettID = SpielID2WettID[spielID]
    calculateRisk(wettID, spielID)

def calculateRiskOfBet(wettID):
    spielID = WettID2SpielID[wettID]
    calculateRisk(wettID, spielID)

def endGame(spielID):
    print("endGame called")
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
    """minDifferenz = 120
    maxDifferenz = 180
    # Schnatz fangen = 150 + Spielende"""

    if standA > standB:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Spielstand"] = 1.2
        Gewinnwahrscheinlichkeit[spielID]["B"]["Spielstand"] = 0.8

        """
        if standA - standB < maxDifferenz and standA - standB > minDifferenz:
            Gewinnwahrscheinlichkeit[spielID]["Even"]["Spielstand"] = 0.3
        else :
            Gewinnwahrscheinlichkeit[spielID]["Even"]["Spielstand"] = 0.0
        """
    elif standB > standA:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Spielstand"] = 0.8
        Gewinnwahrscheinlichkeit[spielID]["B"]["Spielstand"] = 1.2

        """
        if standB - standA < maxDifferenz and standB - standA > minDifferenz:
            Gewinnwahrscheinlichkeit[spielID]["Even"]["Spielstand"] = 0.3
        else :
            Gewinnwahrscheinlichkeit[spielID]["Even"]["Spielstand"] = 0.0
        """
    """
    else 
        Gewinnwahrscheinlichkeit[spielID]["Even"]["Spielstand"] = 0.0
    """

def verletzung(spielID, team):
    if team == 1:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Verletzung"] -= 0.2
        Gewinnwahrscheinlichkeit[spielID]["B"]["Verletzung"] += 0.2
    else:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Verletzung"] += 0.2
        Gewinnwahrscheinlichkeit[spielID]["B"]["Verletzung"] -= 0.2

def freiwurf(spielID, team):
    if team == 1:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Freiwurf"] += 0.1
        Gewinnwahrscheinlichkeit[spielID]["B"]["Freiwurf"] -= 0.1
    else:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Freiwurf"] -= 0.1
        Gewinnwahrscheinlichkeit[spielID]["B"]["Freiwurf"] += 0.1

def platzverweis(spielID, anzahlTeamA, anzahlTeamB):
    if anzahlTeamA > anzahlTeamB:
        diff = anzahlTeamA - anzahlTeamB
        Gewinnwahrscheinlichkeit[spielID]["A"]["Platzverweis"] -= diff * 0.3
        Gewinnwahrscheinlichkeit[spielID]["B"]["Platzverweis"] += diff * 0.3
    elif anzahlTeamA < anzahlTeamB:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Platzverweis"] += diff * 0.3
        Gewinnwahrscheinlichkeit[spielID]["B"]["Platzverweis"] -= diff * 0.3
    else :
        Gewinnwahrscheinlichkeit[spielID]["A"]["Platzverweis"] = 0.0
        Gewinnwahrscheinlichkeit[spielID]["B"]["Platzverweis"] = 0.0

def dobby(spielID, team):
    if team == 1:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Dobby"] -= 0.2
        Gewinnwahrscheinlichkeit[spielID]["B"]["Dobby"] += 0.2
    else:
        Gewinnwahrscheinlichkeit[spielID]["A"]["Dobby"] += 0.2
        Gewinnwahrscheinlichkeit[spielID]["B"]["Dobby"] -= 0.2

def message_handling(client, userdata, msg):
    print(f"{msg.topic}: {msg.payload.decode()}")

    match msg.topic:
        case "neuesSpiel":
            print("match case neues Spiel")
            spielID = int( msg.payload.decode().split(":")[0] )
            newBetOption(spielID)

        case "spielBeginn":
            print("match case spielBeginn")
            spielID = int( msg.payload.decode().split(":")[0] )
            SpielIsStarted[spielID] = True
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
            print("x")

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

try:
    print("Press CTRL+C to exit...")
    client.loop_forever()
except Exception:
    print("Caught an Exception, something went wrong...")
finally:
    print("Disconnecting from the MQTT broker")
    client.disconnect()