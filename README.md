# iot_mesh_smarthome

### Introduction:

This guide demonstrates setting up a mesh network using painless mesh with ESP8266, dividing it into master and slave nodes. The mesh network is established, and temperature, humidity, and light data are transmitted to Home Assistant (Hass) through MQTT communication with a Raspberry Pi. Conversely, commands received from Hass can be relayed to individual slave nodes.

### libraries (using the library manager):
1. Install painlessMesh.
2. Install Arduino_JSON (although JSON might be installed automatically with painlessMesh, double-checking is advisable).
3. Install PubSubClient (only necessary for the master node).

### Usage:
1. Power up the Raspberry Pi and prepare to receive slave node data via MQTT through the master node by using the following command:
  mosquitto_sub -h 203.252.106.154 -t "iot/mesh/#" -u "iot" -P "csee1414"
2. In the 'mesh_data.ino' file, slave node names are configured. If you want to connect multiple slave nodes to a master node, adjust the value of int nodeNumber = (num); in the file. Ensure that each slave node has a unique value. The default value in the 'mesh_data.ino' file is 3. ('mesh_data.ino' => slave node, 'mqttBridge' => master node)
3. To enable both station and access point modes on the ESP8266, modify 'painlessMeshSTA.cpp' within the painlessMesh library. 
  Change line 56 from WiFi.scanNetworks(true, true, mesh->_meshChannel); to WiFi.scanNetworks(true, true);.
4. After running Hass on the Raspberry Pi, you can observe changing values on the Hass webpage. To integrate the changes, update the '~/.homeassistant/configuration.yaml' file, which corresponds to the 'configuration.yaml' file in the Design section.

By following these steps, you can establish a mesh network, communicate data between master and slave nodes, and interact with Home Assistant for monitoring and control.

### libraries:
- DHT22 - DHT sensor library for ESPx
- OLED – adafruit_SSD1306, adafruit_gfx
- PainlessMash.h
- PubSubClient.h
- Arduino_JSON.h

### Device:
- LED
- DHT22
- Relay(USBLED)
- CDS
- ESP8266


### Result
#### master node
![image](https://github.com/jacobkim98/iot_mesh_smarthome/assets/142479612/9c9e35d8-a861-4bec-b557-8ee6f52ce163)

#### slave node
![image](https://github.com/jacobkim98/iot_mesh_smarthome/assets/142479612/cdd54be9-865d-4ffb-bd4c-51bdd505bf3e)

#### mqtt
![image](https://github.com/jacobkim98/iot_mesh_smarthome/assets/142479612/bdd4de57-cfaa-4c4c-a52d-b4793a68d4d6)

#### hass
![image](https://github.com/jacobkim98/iot_mesh_smarthome/assets/142479612/0568dabb-3ca2-4066-9b29-ba4acbef615e)
