#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <DallasTemperature.h>
#include <OneWire.h>

/*
 * Inside your MyConfig.h file, you will need to define the params to connect
 * to your Wifi and MQTT server:
 * 
 * #define WIFI_SSID "Your SSID"
 * #define WIFI_PWD "Your-Password"
 * #define MQTT_SERVER "your.server.com"
 * #define MQTT_PORT your-mqtt-port
 * #define MQTT_CLIENT_ID "your-client-id"
 * #define MQTT_USERNAME "your-mqtt-username"
 * #define MQTT_PWD "your-mqtt-password
 * #define SENSOR_SUB_NAME "your-sensor-name"
 * #define SENSOR_LOCATION_DESCRIPTION "Your Sensor Description"
 */
#include "MyConfig.h"
 
#define ONE_WIRE_BUS 2 // Pin where dallas sensor is connected 
#define MESSAGE_INTERVAL 30000
#define METRIC false

#define TEMP_TYPE "Temperature"
#define TEMP_DESC "Temperature"

WiFiClient espClient;
PubSubClient client(espClient);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

long lastMsgSent = 0;

void setup()
{
  startWifi();
 
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setKeepAlive((MESSAGE_INTERVAL / 1000) * 2);
  client.setBufferSize(512);

  sensors.begin();
}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }

  long now = millis();
  long millisUntilNextSample = (lastMsgSent + MESSAGE_INTERVAL) - now;
  if (millisUntilNextSample <= 0) {
    lastMsgSent = now;

    sensors.requestTemperatures();

    float temperature = static_cast<float>(static_cast<int>(
      (METRIC ? sensors.getTempCByIndex(0) : sensors.getTempFByIndex(0)) * 10.)) / 10.;
 
    if (temperature != 85.00 && temperature > -32.0) {
      char tempStateTopic[64];
      getTempStateTopic(tempStateTopic, sizeof(tempStateTopic));

      client.publish(tempStateTopic, String(temperature).c_str());
    }
  } else {
    delay(millisUntilNextSample);
  }
}

void reconnect() {
  char topic[64];
  char msg[256];
  
  getAvailabilityTopic(topic, sizeof(topic));

  while (!client.connected()) {
#ifdef MQTT_USERNAME    
    client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, topic, 0, true, "offline");
#else
    client.connect(MQTT_CLIENT_ID, topic, 0, true, "offline");
#endif
  }

  getTempConfigTopic(topic, sizeof(topic));
  getTempConfigMessage(msg, sizeof(msg));
  client.publish(topic, msg, true);
  
  getAvailabilityTopic(topic, sizeof(topic));
  client.publish(topic, "online", true);
}

void startWifi() {
  delay(50);

  // Ensure that ESP8266 only starts up in Station mode
  WiFi.mode(WIFI_STA);

  // Save some power, produce less corrupting heat
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);

  // Set the hostname, reusing the mqtt client id
  WiFi.hostname(MQTT_CLIENT_ID);

  WiFi.begin(WIFI_SSID, WIFI_PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

int getAvailabilityTopic(char* dest, int n) {
  return snprintf(dest, n, "iot/%s%s/availability", SENSOR_SUB_NAME, TEMP_TYPE);
}

int getTempConfigTopic(char* dest, int n) {
  return getConfigTopic(dest, n, TEMP_TYPE);
}

int getConfigTopic(char* dest, int n, const char* type) {
  return snprintf(dest, n, "homeassistant/sensor/%s%s/config", SENSOR_SUB_NAME, type);
}

int getTempConfigMessage(char* dest, int n) {
  return getConfigMessage(dest, n, TEMP_DESC, TEMP_TYPE, "\"unit_of_measurement\": \"°F\"");
}

int getConfigMessage(char* dest, int n, const char* typeDesc, const char* type, const char* typeSpecific) {
  char availabilityTopic[64];
  getAvailabilityTopic(availabilityTopic, sizeof(availabilityTopic));

  char stateTopic[64];
  getStateTopic(stateTopic, sizeof(stateTopic), type);
  
  return snprintf(dest, n,
            "{\"name\": \"%s %s\", \"state_topic\": \"%s\", \"availability_topic\": \"%s\", %s}",
            SENSOR_LOCATION_DESCRIPTION, typeDesc, stateTopic, availabilityTopic, typeSpecific);
}

int getTempStateTopic(char* dest, int n) {
    return getStateTopic(dest, n, TEMP_TYPE);
}

int getStateTopic(char* dest, int n, const char* type) {
  return snprintf(dest, n, "iot/%s%s/state", SENSOR_SUB_NAME, type);
}
