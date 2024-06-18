import sys
import paho.mqtt.client as paho

TEAM_A = 1
TEAM_B = 2
EVEN   = 3
minQuote = 3.5

client = paho.Client()

def isBetOptionInteresting(quoteA, quoteB, quoteEven):
    global minQuote
    global TEAM_A
    global TEAM_B
    global EVEN

    if quoteA > minQuote and quoteA > quoteB:
        return True, TEAM_A
    if quoteB > minQuote:
        return True, TEAM_B
    if quoteEven > minQuote:
        return True, EVEN
    return False, 0

def newBet(message):
    anbieterName = msg.payload.decode().split(":")[0]
    wettID = int( msg.payload.decode().split(":")[1] )
    quoteA = float( msg.payload.decode().split(":")[2] )
    quoteB = float( msg.payload.decode().split(":")[3] )
    quoteEven = float( msg.payload.decode().split(":")[4] )
    # anbieterName + ":" + wettIDctr + ":" quoteA + ":" + quoteB + ":" + quoteEven + ":"
    betOptionIsInteresting, ending = isBetOptionInteresting(quoteA, quoteB, quoteC)
    
    if betOptionIsInteresting:
        '''publish_client = paho.Client()
        if publish_client.connect("mqttbroker", 1883, 60) != 0:
        print("Couldn't connect to the mqtt broker")
        sys.exit(1)

        mqtt_message = anbieterName + ":" + wettID + ":" + ending + ":" + moneyset

        #publish_client.publish("neueAutowette", "neue Autowette", 0)
        
        publish_client.disconnect()'''
    

def message_handling(client, userdata, msg):
    print(f"{msg.topic}: {msg.payload.decode()}")
    newBet(msg.topic)


client = paho.Client()
client.on_message = message_handling

if client.connect("mqttbroker", 1883, 60) != 0:
    print("Couldn't connect to the mqtt broker")
    sys.exit(1)

client.subscribe("neuesWettangebot")

try:
    print("Press CTRL+C to exit...")
    client.loop_forever()
except Exception:
    print("Caught an Exception, something went wrong...")
finally:
    print("Disconnecting from the MQTT broker")
    client.disconnect()