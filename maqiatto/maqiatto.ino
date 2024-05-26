#include <ESP8266WiFi.h>
#include <Arduino.h>
#include "PubSubClient.h"
#include "Credentials.h"

#define WIFI_TIMEOUT 1000

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

boolean mqttInitCompleted = false;
String clientId = "ianterzo.iot.esp.id" + String(ESP.getChipId());

void dataCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print(length);
  if (strcmp(topic, TOPIC_MOVEMENT) == 0) {
    Serial.print(length);
    if (length == 2) { // Se om paketer är 2 bytes
      byte byte1 = payload[0];
      byte byte2 = payload[1]; 

      // Extrahera boolearna
      boolean boolean1 = (byte1 >> 1) & 1;
      boolean boolean2 = byte1 & 1; 

      // Extrahera integern
      int integer = byte2;

      Serial.print("boolean1: ");
      Serial.println(boolean1);
      Serial.print("boolean2: ");
      Serial.println(boolean2);
      Serial.print("integer: ");
      Serial.println(integer);
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
  if(mqttInitCompleted)
  {
    if (!mqttClient.connected())
    {
      performConnect();
    }
    mqttClient.loop();

    if(MQTTPublish(TOPIC_SENSOR, "test maqiatto!"))
    {
      Serial.printf("MQTTPublish was succeeded.\n");
    }
  }

 
}
