#!/usr/bin/env python

# This client demonstrates Thrift's connection and "oneway" asynchronous jobs
# Client connects to server host:port and calls 2 methods
# showCurrentTimestamp : which returns current time stamp from server
# asynchronousJob() : which calls a "oneway" method
#
# source: https://cloudxlab.com/blog/creating-thrift-service/

host = "p2-socket"
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
        print("beenden? :4 \n")

        input2 = int(input("Eingabe: "))

        if input2 == 1:

            pid = int(input("ID?: "))
            playera = input("Player A?: ")
            playerb = input("Player B?: ")
            wina = float(input("Chance A?: "))
            winb = float(input("Chance B?: "))
            even = float(input("Even?: "))

            result = client.addBet(pid, playera, playerb, wina, winb, even)
            print (result)

        elif input2 == 2:

            pid = int(input("ID?: "))
            playera = input("Player A?: ")
            playerb = input("Player B?: ")
            wina = float(input("Chance A?: "))
            winb = float(input("Chance B?: "))
            even = float(input("Even?: "))

            result = client.changeBet(pid, playera, playerb, wina, winb, even)
            print (result)

        elif input2 == 3:

            pid = int(input("ID?: "))
            playera = input("Player A?: ")
            playerb = input("Player B?: ")
            wina = float(input("Chance A?: "))
            winb = float(input("Chance B?: "))
            even = float(input("Even?: "))

            result = client.deleteBet(pid, playera, playerb, wina, winb, even)
            print (result)
            
        elif input2 == 4:
            loop = False
    

    # Close connection
    transport.close()

except Thrift.TException as tx:
    print ('Something went wrong : %s' % (tx.message))