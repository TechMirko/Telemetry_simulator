import paho.mqtt.client as mqtt
import json

import matplotlib
matplotlib.use('TkAgg') # Forza l'uso di finestre interattive
import matplotlib.pyplot as plt

rpms = []
temps = []

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
        dati = json.loads(payloadString)
        if "RPM" in msg.topic:
            rpms.append(dati['val'])
        else:
            temps.append(dati['val'])
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

print(f"RPM: {rpms}")
print(f"TEMPS: {temps}")

plt.plot(rpms, color='black')
plt.title("Andamento RPM nel giro")
plt.grid()
plt.xlabel("Tempo nel giro")
plt.ylabel("RPM")
plt.show()

plt.plot(temps, color='orange')
plt.title("Andamento temperatura motore nel giro")
plt.grid()
plt.xlabel("Tempo nel giro")
plt.ylabel("°C")
plt.show()