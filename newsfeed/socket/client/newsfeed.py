host = "p2-socket"
port = 9090

import sys
import paho.mqtt.client as paho

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

#try:
loop = True 
while loop:

    print("Neues Spiel? :1 \n")
    print("Spielbeginn? :2 \n")
    print("Spielende? :3 \n")
    print("aktueller Stand? :4 \n")
    print("Verletzung? :5 \n")
    print("Freiwurf? :6 \n")
    print("Platzverweis? :7 \n")
    print("Dobbys Klatscher? :8 \n")
    print("beenden? :9 \n")

    input2 = int(input("Eingabe: "))

    if input2 == 1:
        pid = input("Spiel-ID?: ")
        playera = input("Player A?: ")
        playerb = input("Player B?: ")

        news = pid + ":" + "Neu" + ":" + playera + ":" + playerb

        client = paho.Client()

        if client.connect("mqttbroker", 1883, 60) != 0:
            print("Couldn't connect to the mqtt broker")
            sys.exit(1)

        client.publish("newsfeed", news, 0)
        client.disconnect()

    elif input2 == 2:
        pid = input("Spiel-ID?: ")

        news = pid + ":" + "Beginn"

        client = paho.Client()

        if client.connect("mqttbroker", 1883, 60) != 0:
            print("Couldn't connect to the mqtt broker")
            sys.exit(1)

        client.publish("newsfeed", news, 0)
        client.disconnect()
        

    elif input2 == 3:

        pid = input("Spiel-ID?: ")
        playera = input("Stand Spieler A?: ")
        playerb = input("Stand Spieler B?: ")

        news = pid + ":" + "Ende" + ":" + playera + ":" + playerb

        client = paho.Client()

        if client.connect("mqttbroker", 1883, 60) != 0:
            print("Couldn't connect to the mqtt broker")
            sys.exit(1)

        client.publish("newsfeed", news, 0)
        client.disconnect()            

    elif input2 == 4:

        pid = input("Spiel-ID?: ")
        playera = input("Stand Spieler A?: ")
        playerb = input("Stand Spieler B?: ")

        news = pid + ":" + "Stand" + ":" + playera + ":" + playerb

        client = paho.Client()

        if client.connect("mqttbroker", 1883, 60) != 0:
            print("Couldn't connect to the mqtt broker")
            sys.exit(1)

        client.publish("newsfeed", news, 0)
        client.disconnect()  
    
    elif input2 == 5:

        pid = input("Spiel-ID?: ")
        team = input("Verletzung Team A(1), B(2)?: ")

        if team == "1" or team == "2":

            news = pid + ":" + "Verletzung" + ":" + team

            client = paho.Client()

            if client.connect("mqttbroker", 1883, 60) != 0:
                print("Couldn't connect to the mqtt broker")
                sys.exit(1)

            client.publish("newsfeed", news, 0)
            client.disconnect()
        else:
            print("invalid input")

    elif input2 == 6:

        pid = input("Spiel-ID?: ")
        team = input("Freiwurf Team A(1), B(2)?: ")

        if team == "1" or team == "2":

            news = pid + ":" + "Freiwurf" + ":" + team

            client = paho.Client()

            if client.connect("mqttbroker", 1883, 60) != 0:
                print("Couldn't connect to the mqtt broker")
                sys.exit(1)

            client.publish("newsfeed", news, 0)
            client.disconnect()
        else:
            print("invalid input")

    elif input2 == 7:

        pid = input("Spiel-ID?: ")
        playera = input("Anzahl Spieler A?: ")
        playerb = input("Anzahl Spieler B?: ")

        news = pid + ":" + "Platzverweis" + ":" + playera + ":" + playerb

        client = paho.Client()

        if client.connect("mqttbroker", 1883, 60) != 0:
            print("Couldn't connect to the mqtt broker")
            sys.exit(1)

        client.publish("newsfeed", news, 0)
        client.disconnect()

    elif input2 == 8:

        pid = input("Spiel-ID?: ")
        team = input("Dobby gegen Team A(1), B(2)?: ")

        if team == "1" or team == "2":

            news = pid + ":" + "DobbyKlatscherGegen" + ":" + team

            client = paho.Client()

            if client.connect("mqttbroker", 1883, 60) != 0:
                print("Couldn't connect to the mqtt broker")
                sys.exit(1)

            client.publish("newsfeed", news, 0)
            client.disconnect()
        else:
            print("invalid input")            
        
    elif input2 == 9:
        loop = False




'''

    # Init thrift connection and protocol handlers
    transport = TSocket.TSocket( host , port)
    transport = TTransport.TBufferedTransport(transport)
    protocol = TBinaryProtocol.TBinaryProtocol(transport)

    # Set client to our Example
    client = Wettanbieter.Client(protocol)

    # Connect to server
    transport.open()  

    # Close connection
    transport.close()

except Thrift.TException as tx:
    print ('Something went wrong : %s' % (tx.message))

'''