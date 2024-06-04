#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <Servo.h>
#include "PubSubClient.h"
#include "Credentials.h"

// Definiera konstanter för WiFi timeout och pinnar
#define WIFI_TIMEOUT 1000

#define PIN_ENA D0
#define PIN_IN1 D1
#define PIN_IN2 D2

#define ECHO_PIN D6
#define TRIG_PIN D7

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

boolean mqttInitCompleted = false;
String clientId = "ianterzo.iot.esp.id" + String(ESP.getChipId());

Servo steering;

long lastDuration = 0;

// Funktion för att styra motorn
void moveMotor(int direction, int speed) {
  if (direction == 0) {
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
  } else if (direction == 1) {
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, HIGH);
  }
  analogWrite(PIN_ENA, speed);
}

// Funktion för att styra servon
void moveServo(int degrees) {
  steering.write(degrees);
}

// Callback-funktion för MQTT-data
void dataCallback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, TOPIC_MOVEMENT) == 0) {
    // Kontrollera om paketet är 2 byte
    if (length == 2) {
      byte byte1 = payload[0];
      byte byte2 = payload[1];

      // Extrahera booleanvärden
      boolean boolean1 = (byte1 >> 1) & 1;
      boolean boolean2 = byte1 & 1;
 
      // Extrahera heltal
      int integer = byte2;

      if (boolean1 == 0) {
        moveMotor(boolean2, integer);
      } else if (boolean1 == 1) {
        moveServo(integer);
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.setDebugOutput(true);

  // Anslut servon till pinne D2
  steering.attach(2);

  // Ställ in motorpinnar som utgångar
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);

  // Ställ in ultraljudssensorpinnar
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Anslut till WiFi-nätverk
  WiFi.begin(STA_SSID, STA_PASS);
  Serial.printf("Waiting for AP connection ...\n");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.printf(".");
  }
  IPAddress ip = WiFi.localIP();
  Serial.printf("\nConnected to AP. IP : %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);

  // Konfigurera MQTT-klient
  mqttClient.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  mqttClient.setCallback(dataCallback);  // Ställ in callback-funktionen
  mqttInitCompleted = true;
}

// Funktion för att ansluta till MQTT
void performConnect() {
  uint16_t connectionDelay = 5000;
  while (!mqttClient.connected()) {
    Serial.printf("Trace   : Attempting MQTT connection...\n");
    if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_KEY)) {
      Serial.printf("Trace   : Connected to Broker.\n");

      // Prenumerera på dina ämnen efter anslutning
      mqttClient.subscribe(TOPIC_MOVEMENT);
      mqttClient.subscribe(TOPIC_SENSOR);
    } else {
      Serial.printf("Error!  : MQTT Connect failed, rc = %d\n", mqttClient.state());
      Serial.printf("Trace   : Trying again in %d msec.\n", connectionDelay);
      delay(connectionDelay);
    }
  }
}

void loop() {
  if (mqttInitCompleted) {
    if (!mqttClient.connected()) {
      performConnect();
    } else {
      // Mät hastigheten med ultraljudsensor
      digitalWrite(TRIG_PIN, LOW);
      delayMicroseconds(2);
      digitalWrite(TRIG_PIN, HIGH);
      delayMicroseconds(15);

      long duration = pulseIn(ECHO_PIN, HIGH);
      Serial.println(duration);
      // Skicka "true" (1) när ett föremål är rakt under sensorn
      if (duration > 400) {
        int value = 1;
        byte converted = (byte)value; // Konvertera heltal till en byte
        mqttClient.publish(TOPIC_SENSOR, &converted, 1);
      }
    }
    mqttClient.loop();
  }
}
