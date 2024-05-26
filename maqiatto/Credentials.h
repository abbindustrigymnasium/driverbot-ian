#ifndef ARDUINO_CREDENTIALS_H
#define ARDUINO_CREDENTIALS_H

/* WiFi Credentials to connect Internet */
#define STA_SSID ""
#define STA_PASS ""

/* Provide MQTT broker credentials as denoted in maqiatto.com. */
#define MQTT_BROKER       "maqiatto.com"
#define MQTT_BROKER_PORT  1883
#define MQTT_USERNAME     "ian.baldelli@gmail.com"
#define MQTT_KEY          ""


/* Provide topic as it is denoted in your topic list. 
 * For example mine is : cadominna@gmail.com/topic1
 * To add topics, see https://www.maqiatto.com/configure
 */
#define TOPIC_MOVEMENT    "ian.baldelli@gmail.com/movement"
#define TOPIC_SENSOR   "ian.baldelli@gmail.com/sensor"

#endif /* ARDUINO_CREDENTIALS_H */
