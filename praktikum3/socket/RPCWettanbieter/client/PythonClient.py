#!/usr/bin/env python

# This client demonstrates Thrift's connection and "oneway" asynchronous jobs
# Client connects to server host:port and calls 2 methods
# showCurrentTimestamp : which returns current time stamp from server
# asynchronousJob() : which calls a "oneway" method
#
# source: https://cloudxlab.com/blog/creating-thrift-service/

host = "localhost"
port = 9090

import sys

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

try:

    # Init thrift connection and protocol handlers
    transport = TSocket.TSocket( host , port)
    transport = TTransport.TBufferedTransport(transport)
    protocol = TBinaryProtocol.TBinaryProtocol(transport)

    # Set client to our Example
    client = Wettanbieter.Client(protocol)

    # Connect to server
    transport.open()

    loop = True

    while loop:

        print("Neue Wette? :1 \n")
        print("Wette anpassen? :2 \n")
        print("Wette entfernen? :3 \n")
        print("Wetten ausgeben? :4 \n")
        print("Wettende auf Wette abfragen? :5 \n")
        print("Wetten? :6 \n")
        print("Wette abfragen? :7 \n")
        print("beenden? :8 \n")

        input2 = int(input("Eingabe: "))

        if input2 == 1:

            name = input("Your Name?: ")
            pid = int(input("Wett-ID?: "))
            gameid = int(input("Spiel-ID?: "))
            wina = float(input("Chance A?: "))
            winb = float(input("Chance B?: "))
            even = float(input("Even?: "))

            result = client.addBet(name, pid, gameid, wina, winb, even)
            print (result)

        elif input2 == 2:

            name = input("Your Name?: ")
            pid = int(input("Wett-ID?: "))
            gameid = int(input("Spiel-ID?: "))
            wina = float(input("Chance A?: "))
            winb = float(input("Chance B?: "))
            even = float(input("Even?: "))

            result = client.changeBet(name, pid, gameid, wina, winb, even)
            print (result)

        elif input2 == 3:

            name = input("Your Name?: ")
            pid = int(input("ID?: "))

            result = client.deleteBet(name, pid)
            print (result)

        elif input2 == 4:

            name = input("Your Name?: ")

            result = client.getMyBetOptions(name)
            print (result)
        
        elif input2 == 5:

            name = input("Your Name?: ")
            pid = int(input("ID?: "))

            result = client.getAllPlacedBetsToABetOption(name, pid)
            print (result)

        elif input2 == 6:

            nameanbieter = input("Name bookmaker?: ")
            pid = int(input("ID?: "))

            result1 = client.getSpecificBetOption(nameanbieter, pid)
            if result1 :
                name = input("Your Name?: ")
                moneyset = double(input("Money?: "))

                ending = int(input("wina (1), winb(2), even (3)?: "))

                arrayResult = result1.split(":")

                wina = float(arrayResult[-3])
                winb = float(arrayResult[-2])
                even = float(arrayResult[-1].removesuffix(';\n'))

                result = client.placeNewBet(nameanbieter, pid, name, moneyset, ending, wina, winb, even)
                print (result)
            else :
                print ("No bet found with this id and bookmaker")

        elif input2 == 7:

            nameanbieter = input("Name bookmaker?: ")
            pid = int(input("ID?: "))

            result1 = client.getSpecificBetOption(nameanbieter, pid)
            print(result1)
            
            
        elif input2 == 8:
            loop = False
    

    # Close connection
    transport.close()

except Thrift.TException as tx:
    print ('Something went wrong : %s' % (tx.message))