import paho.mqtt.client as mqtt

# Connection callback function
def onConnect(client, userData, flags, rc, properties):
    if rc == 0:
        print(f"[PYTHON] Connected to the broker MQTT!")
        client.subscribe("fsae/car/#")
    else:
        print(f"[PYTHON] Error on connection. Code {rc}")

# Message receiving callback fuction
def onMessageReceived(client, userData, msg):
    try:
        # Decodifica il payload
        payloadString = msg.payload.decode('utf-8')
        print(f"TOPIC: {msg.topic} | PAYLOAD: {payloadString}")
    except Exception as e:
        print(f"[ERROR] Decoding payload: {e}")

# -- MAIN -- #
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = onConnect
client.on_message = onMessageReceived

# Connection
print(f"[PYTHON] Waiting connection...")
client.connect("localhost", 1883, 60)

# Infinite loop who handle the network and mantain the script alive
try:
    client.loop_forever()
except KeyboardInterrupt:
    print("\n[PYTHON] Disconnecting...")
    client.disconnect()