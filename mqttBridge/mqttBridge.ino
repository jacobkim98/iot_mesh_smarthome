//************************************************************
// this is a simple example that uses the painlessMesh library to
// connect to a another network and relay messages from a MQTT broker to the nodes of the mesh network.
// To send a message to a mesh node, you can publish it to "painlessMesh/to/12345678" where 12345678 equals the nodeId.
// To broadcast a message to all nodes in the mesh you can publish it to "painlessMesh/to/broadcast".
// When you publish "getNodes" to "painlessMesh/to/gateway" you receive the mesh topology as JSON
// Every message from the mesh which is send to the gateway node will be published to "painlessMesh/from/12345678" where 12345678 
// is the nodeId from which the packet was send.
//************************************************************

#include <Arduino.h>
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>

#define   MESH_PREFIX     "khj"
#define   MESH_PASSWORD   "12341234"
#define   MESH_PORT       5555

#define   STATION_SSID     "Jacob's"
#define   STATION_PASSWORD "12341234"

#define HOSTNAME "MQTT_Bridge"

//String topic_temp_2 = "iot/mesh/2/dht22_t"
//String topic_temp_3 = "iot/mesh/3/dht22_t"
//String topic_hum_2 = "iot/mesh/2/dht22_h"
//String topic_hum_3 = "iot/mesh/3/dht22_h"
//String topic_cds_2 = "iot/mesh/2/cds"
//String topic_cds_3 = "iot/mesh/3/cds"


// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);

IPAddress getlocalIP();

IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(203, 252, 106, 154);

painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);

int node;
double temp;
double hum;
int cds;
char str_temperature[40], str_humidity[40], str_light_intensity[40];

void setup() {
  Serial.begin(115200);
  
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 3 );
  mesh.onReceive(&receivedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
}

void loop() {
  mesh.update();
  mqttClient.loop();

  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

    
    if (mqttClient.connect("painlessMeshClient", "iot", "csee1414")) {
      mqttClient.publish("iot/123","Ready!");
      mqttClient.subscribe("iot/mesh");
    } 
  }
}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  delay(1000);
  
  
  
  //  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
//  String topic = "painlessMesh/from/" + String(from);
////  mqttClient.publish(topic.c_str(), msg.c_str());
//  mqttc.publish("iot/21901019/cds", msg.c_str());
//  delay(500);
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  JSONVar myObject = JSON.parse(msg.c_str());
  node = myObject["node"];
  temp = myObject["temp"];
  hum = myObject["hum"];
  cds = myObject["cds"];
  Serial.print("Node: ");
  Serial.println(node);
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");
  Serial.print("Light value: ");
  Serial.print(cds);
  Serial.println(" (0 ~ 1024)");

  String topic_t = "iot/mesh/" + String(node) + "/dht22_t";
  String topic_h = "iot/mesh/" + String(node) + "/dht22_h";
  String topic_c = "iot/mesh/" + String(node) + "/cds";

  sprintf(str_temperature, "%3.1f", temp);
  sprintf(str_humidity, "%3.1f", hum);
  sprintf(str_light_intensity, "%d", cds);

//  mqttc.publish("iot/123", node);
  mqttClient.publish(topic_t.c_str(), str_temperature);
  mqttClient.publish(topic_h.c_str(), str_humidity);
  mqttClient.publish(topic_c.c_str(), str_light_intensity);

  delay(3000);
  
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  char* cleanPayload = (char*)malloc(length+1);
  memcpy(cleanPayload, payload, length);
  cleanPayload[length] = '\0';
  String msg = String(cleanPayload);
  free(cleanPayload);

  String targetStr = String(topic).substring(16);

  char message_buff[100]; //initialise storage buffer
  String msgString;
  int i = 0;
  
  for(i=0; i<length; i++){
    message_buff[i] = payload[i];
  }
  message_buff[i]= '\0';
  msgString = String(message_buff );

  if(msgString == "2_usbledon"){
    mesh.sendBroadcast(msgString);
    Serial.println("Message : usbledon");
  }else if(msgString == "2_usbledoff"){
    mesh.sendBroadcast(msgString);
    Serial.println("Message : usbledoff");
  }else if(msgString == "3_usbledon"){
    mesh.sendBroadcast(msgString);
    Serial.println("Message : usbledon");
  } else if(msgString == "3_usbledoff"){
    mesh.sendBroadcast(msgString);
    Serial.println("Message : usbledoff");
  }




  
  if(targetStr == "gateway")
  {
    if(msg == "getNodes")
    {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish("iot/123", str.c_str());
    }
  }
  else if(targetStr == "broadcast") 
  {
    mesh.sendBroadcast(msg);
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if(mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg);
    }
    else
    {
//      mqttClient.publish("iot/mesh", "Client not connected!");
    }
  }
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
