#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <Servo.h>

#include "PubSubClient.h"
#include "Credentials.h"

#define WIFI_TIMEOUT 1000

#define PIN_ENA D0  // The ESP8266 pin connected to the EN1 pin L298N
#define PIN_IN1 D1  // The ESP8266 pin connected to the IN1 pin L298N
#define PIN_IN2 D2  // The ESP8266 pin connected to the IN2 pin L298N

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

boolean mqttInitCompleted = false;
String clientId = "ianterzo.iot.esp.id" + String(ESP.getChipId());

Servo steering;

void moveMotor(int direction, int speed){

  if (direction == 0){
    digitalWrite(PIN_IN1, HIGH);   // control the motor's direction in anti-clockwise
    digitalWrite(PIN_IN2, LOW);  // control the motor's direction in anti-clockwise
  }
  else if (direction == 1){
    digitalWrite(PIN_IN1, LOW);   // control the motor's direction in anti-clockwise
    digitalWrite(PIN_IN2, HIGH);  // control the motor's direction in anti-clockwise
  }
  
  analogWrite(PIN_ENA, speed);
}

void moveServo(int degrees){
  steering.write(degrees);
}

void dataCallback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, TOPIC_MOVEMENT) == 0) {
    if (length == 2) { // Se om paketer är 2 bytes
      byte byte1 = payload[0];
      byte byte2 = payload[1]; 

      // Extrahera boolearna
      boolean boolean1 = (byte1 >> 1) & 1;
      boolean boolean2 = byte1 & 1; 
      Serial.println(boolean1);
       Serial.println(boolean2);
       
      // Extrahera integern
      int integer = byte2;
       Serial.println(integer);

      if (boolean1 == 0){
        moveMotor(boolean2, integer);
      }
      else if (boolean1 == 1)
        moveServo(integer);
    }
  }
}


boolean MQTTPublish(const char* topic, char* payload)
{
  boolean retval = false;
  if (mqttClient.connected())
  {
    retval = mqttClient.publish(topic, payload);
  }
  return retval;
}

void setup() 
{ 
  Serial.begin(9600);
  Serial.setDebugOutput(true);

  steering.attach(2);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);

  WiFi.begin(STA_SSID, STA_PASS);
  Serial.printf("Waiting for AP connection ...\n");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(WIFI_TIMEOUT);
    Serial.printf(".");
  }
  IPAddress ip = WiFi.localIP();
  Serial.printf("\nConnected to AP. IP : %d.%d.%d.%d\n", ip[0],ip[1],ip[2],ip[3]);

  mqttClient.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  mqttClient.setCallback(dataCallback);  // This sets the callback function
  mqttInitCompleted = true;
}

void performConnect()
{
  uint16_t connectionDelay = 5000;
  while (!mqttClient.connected())
  {
    Serial.printf("Trace   : Attempting MQTT connection...\n");
    if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_KEY))
    {
      Serial.printf("Trace   : Connected to Broker.\n");

      // Subscription to your topic after connection was succeeded.
      mqttClient.subscribe(TOPIC_MOVEMENT);
      mqttClient.subscribe(TOPIC_SENSOR);
    }
    else
    {
      Serial.printf("Error!  : MQTT Connect failed, rc = %d\n", mqttClient.state());
      Serial.printf("Trace   : Trying again in %d msec.\n", connectionDelay);
      delay(connectionDelay);
    }
  }
}

void loop() 
{
  //steering.write(90);
  if(mqttInitCompleted)
  {
    if (!mqttClient.connected())
    {
      performConnect();
    }
    mqttClient.loop();

    MQTTPublish(TOPIC_SENSOR, "test maqiatto!");
  }

 
}
