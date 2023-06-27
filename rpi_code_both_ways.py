import paho.mqtt.client as mqtt
import serial
import time
import json

noOfNodes=0
tmp=0

ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
ser.reset_input_buffer()

def intoDecimal(string):
    decimal = int(string, base=16)
    return decimal

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("MQTT Connection OK.")
    else:
        print(f"Connected fail with code {rc}")
    client.subscribe("Iqrf/DpaResponse1")
    jsonrequest=json.dumps({"mType": "iqrfRaw","data": {"msgId": "testRaw","timeout": 1000,"req": {"rData": "00.00.00.00.FF.FF"}}})
    client.publish("Iqrf/DpaRequest1",jsonrequest)

def on_message(client, userdata, msg):
	global tmp
	global noOfNodes
	if tmp==0:
		noOfNodes = intoDecimal(((msg.payload.decode("utf-8").split('"rData": "',1)[1]).split('"',1)[0])[24:26])
		print("Number of nodes in IQRF network is",noOfNodes)
		tmp=1
	else:
		mqttmessage = msg.payload.decode("utf-8")
		request=(mqttmessage.split('"request": "',1)[1]).split('"',1)[0]
		response=(mqttmessage.split('"response": "',1)[1]).split('"',1)[0]
		segments = response.split(".")
		for i in range(len(segments) - 1, -1, -1):
    			if segments[i] != "00":
       				break
		short_response = ".".join(segments[:i+1])
		finalMessage =str(noOfNodes)+("_")+request+("_")+ short_response
		print ("Data from IQRF network:")
		print (finalMessage)
		ser.write(finalMessage.encode('utf-8'))

client = mqtt.Client()
client.on_connect = on_connect
client.on_pre_connect = None
client.on_message = on_message
client.connect("127.0.0.1", 1883, 60)



while (True):
	client.loop(.1)
	if(ser.inWaiting()>0):
		data_str = ser.readline(ser.inWaiting()).decode('ascii')
		if (data_str.startswith("requestlora_")):
			data_str = data_str.replace("requestlora_","")
			tmprequest1 = '{"mType": "iqrfRaw", "data": {"msgId": "testRaw", "timeout": 1000, "req": {"rData": "'
			tmprequest2 ='"},"returnVerbose": true}}'
			tmprequest = tmprequest1 + data_str.strip() + tmprequest2
			client.publish("Iqrf/DpaRequest1",tmprequest)
		else:
			print(data_str)

