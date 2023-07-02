# test_connect.py
import paho.mqtt.client as mqtt
import serial
import time
from datetime import datetime

numberOfNodes=2
tmp=0

ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
ser.reset_input_buffer()

def Convert(string):
    li = list(string.split("."))
    return li

def intoDecimal(string):
    temperature = int(string, base=16)
    temperature = isNegative(temperature) 
    return temperature

def isNegative(int):
   if int >= 200:
      int = int - 256
   return int

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected success")
    else:
        print(f"Connected fail with code {rc}")
    client.subscribe("Iqrf/DpaResponse")

def on_message(client, userdata, msg):
    tmp=0
    finalString = ""
    temp = msg.payload.decode("utf-8")
    sepBeginning = '"rData": "'
    strippedBeginning = temp.split(sepBeginning, 1)[1]
    sepEnd = '"'
    strippedEnd = strippedBeginning.split(sepEnd,1) [0]
    temperatureFromAllNodesString = strippedEnd[30:]
    temperatureFromAllNodesList = Convert(temperatureFromAllNodesString)

    for node in temperatureFromAllNodesList:
       temperature = intoDecimal(node)
       finalString += str(temperature)
       finalString += "."
       tmp=tmp+1
       if tmp==numberOfNodes:
           break

    print(finalString)
    ser.write(finalString.encode('utf-8'))
client = mqtt.Client()
client.on_connect = on_connect
client.on_pre_connect = None
client.on_message = on_message
client.connect("127.0.0.1", 1883, 60)
client.loop_forever()
