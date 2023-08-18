/*
iot lab_2 final project
7조 mesh_data reciver

node number 2
sensor - dht, cds, usb_led(relay), 
*/
#include "painlessMesh.h"
#include "DHTesp.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino_JSON.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//GPIO Pin Mapping 부분이다.
#define D0 16 //wakeup
#define D1 5 //OLED SCL
#define D2 4 //OLED SDA
#define D3 0 //DHT22
#define D4 2 //Relay
#define D5 14 //LED
#define D6 12
#define D7 13
#define D8 15
#define LED_PIN D0
#define LED_ON HIGH // active-HIGH
#define LED_OFF LOW

// 과제를 위한 맵핑 부분
#define LIGHT_PIN A0 // Light 핀 맵핑
#define LED_PIN D5 // LED 핀 맵핑
#define DHTPIN D3 // DHT 핀 맵핑
#define RELAY1_PIN D4 // HW1을 위해 D4로 맵핑 변경
#define RELAY_OFF HIGH // HIGH 신호이면 릴레이 동작하지 않음
#define RELAY_ON LOW // LOW 신호이면 릴레이 동작함

// MESH Details
#define   MESH_PREFIX     "khj"
#define   MESH_PASSWORD   "12341234"
#define   MESH_PORT       5555

//BME object on the default I2C pins
//Adafruit_BME280 bme;
DHTesp dht; // dht 

//Number for this node
int nodeNumber = 3;

// led, usbled의 상태를 위한 함수
int led_state = LED_OFF; //led_state는 led의 상태
int usb_led_state = RELAY_OFF; // 초기 상태는 불 꺼진 상태
bool usbled_state = false;


//정보를 받은 값 저장하는 변수
int node;
int led_state_receive;
int usb_led_state_receive;

// led, usbled 변화 bool 값
bool led_change=false;
bool usb_led_change=false;

//String to send to other nodes with sensor readings
String readings;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings

//Create tasks: to send messages and get readings;
Task taskSendMessage(TASK_SECOND * 20 , TASK_FOREVER, &sendMessage);

String getReadings () {
  JSONVar jsonReadings;
  jsonReadings["node"] = nodeNumber;
  jsonReadings["temp"] = dht.getTemperature();
  jsonReadings["hum"] = dht.getHumidity();
  jsonReadings["cds"] = analogRead(LIGHT_PIN);
//  jsonReadings["usb_led"] = usb_led_state;
//  jsonReadings["led"] = led_state;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}


// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  
  if(msg == "2_usbledon"){
    Serial.println("Message : 2_usbledon");
  }else if(msg == "2_usbledoff"){
    Serial.println("Message : 2_usbledoff");
  }else if(msg == "3_usbledon"){
    digitalWrite(RELAY1_PIN, 0);
    Serial.println("Message : 3_usbledon");
  }else if(msg == "3_usbledoff"){
    digitalWrite(RELAY1_PIN, 1);
    Serial.println("Message : 3_usbledoff");
  }
  
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  // 회로 초기 설정
  dht.setup(DHTPIN, DHTesp::DHT22);
  pinMode(LIGHT_PIN, INPUT);
  pinMode (LED_PIN, OUTPUT);
  pinMode (RELAY1_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, RELAY_OFF);
  
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 3);
  mesh.setContainsRoot(true);
  
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}
