import sys
import paho.mqtt.client as paho

# https://medium.com/@potekh.anastasia/a-beginners-guide-to-mqtt-understanding-mqtt-mosquitto-broker-and-paho-python-mqtt-client-990822274923

client = paho.Client()

client.will_set("lastWill", "I'm gone", qos=0, retain=False)

if client.connect("localhost", 1883, 60) != 0:
    print("Couldn't connect to the mqtt broker")
    sys.exit(1)

client.publish("topic2", "Hi, paho mqtt client works fine!", 0)
#client.disconnect()