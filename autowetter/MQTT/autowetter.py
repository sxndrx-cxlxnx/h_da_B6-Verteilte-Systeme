import sys
import paho.mqtt.client as paho

TEAM_A = 1
TEAM_B = 2
EVEN   = 3
minQuote = 3.5
money = 200

client = paho.Client()

def isBetOptionInteresting(quoteA, quoteB, quoteEven):
    print("isBetOptionInteresting() called)")
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
    print("newBet() called)")
    anbieterName = message.split(":")[0]
    wettID = int( message.split(":")[1] )
    quoteA = float( message.split(":")[2] )
    quoteB = float( message.split(":")[3] )
    quoteEven = float( message.split(":")[4] )
    # anbieterName + ":" + wettIDctr + ":" quoteA + ":" + quoteB + ":" + quoteEven + ":"

    betOptionIsInteresting, ending = isBetOptionInteresting(quoteA, quoteB, quoteEven)
    if betOptionIsInteresting:
        publish_client = paho.Client()
        if publish_client.connect("mqttbroker", 1883, 60) != 0:
            print("Couldn't connect to the mqtt broker")
            sys.exit(1)

        mqtt_message = anbieterName +  ":" + str(wettID) + ':' + "automatischeWette" + ":" + str(money) + ":" + str(ending)  + ':' + str(quoteA) + ':' + str(quoteB) + ':' + str(quoteEven) + ':'

        publish_client.publish("neueAutowette", mqtt_message, 0)
        
        publish_client.disconnect()
        print("New Bet placed")
    

def message_handling(client, userdata, msg):
    print("message_handling() called)")
    print(f"{msg.topic}: {msg.payload.decode()}")
    newBet(msg.payload.decode())


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