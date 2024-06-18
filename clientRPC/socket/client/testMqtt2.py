import sys
import paho.mqtt.client as paho
from datetime import datetime

# https://medium.com/@potekh.anastasia/a-beginners-guide-to-mqtt-understanding-mqtt-mosquitto-broker-and-paho-python-mqtt-client-990822274923

id = 0
start = 0
end = 0

def publish():
    global id
    global start

    client = paho.Client()

    if client.connect("mqttbroker", 1883, 60) != 0:
        print("Couldn't connect to the mqtt broker")
        sys.exit(1)

    start = datetime.now()

    client.publish("testing2", id, 0)
    id +=1
    client.disconnect()


def message_handling(client, userdata, msg):
    global end
    end = datetime.now()
    print(f"{msg.topic}: {msg.payload.decode()}")
    print(msg.payload.decode(), start, ":", end, ':', (end - start).total_seconds() , "s")
    publish()


client = paho.Client()
client.on_message = message_handling

if client.connect("mqttbroker", 1883, 60) != 0:
    print("Couldn't connect to the mqtt broker")
    sys.exit(1)

client.subscribe("testing2")
publish()

try:
    print("Press CTRL+C to exit...")
    client.loop_forever()
except Exception:
    print("Caught an Exception, something went wrong...")
finally:
    print("Disconnecting from the MQTT broker")
    client.disconnect()