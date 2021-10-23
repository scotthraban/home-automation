#include <HWiFi.h>
#include <HMqtt.h>
#include <HOpenHab.h>

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
 * #define SENSOR_NAME "your-sensor-name"
 * #define SENSOR_DESCRIPTION "Your Sensor Description"
 */
#include "MyConfig.h"
 
#define ONE_WIRE_BUS 2 // Pin where dallas sensor is connected 
#define MESSAGE_INTERVAL 30000
#define METRIC false

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

HWiFi hWifi(WIFI_SSID, WIFI_PWD, MQTT_CLIENT_ID);
HMqtt hMqtt(MQTT_SERVER, MQTT_PORT, true, (MESSAGE_INTERVAL / 1000) * 2,
            MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);

long lastMsgSent = 0;

void setup()
{
  sensors.begin();

  hWifi.start();

  char willTopic[64];
  getAvailabilityTopic(willTopic, sizeof(willTopic));
  hMqtt.setWill(willTopic, 0, true, "offline"); 

  hMqtt.start();
}
 
void loop()
{
  long now = millis();
  long millisUntilNextSample = (lastMsgSent + MESSAGE_INTERVAL) - now;
  if (millisUntilNextSample <= 0 && hMqtt.ensureConnected(onConnect)) {
    lastMsgSent = now;

    sensors.requestTemperatures();

    float temperature = static_cast<float>(static_cast<int>(
      (METRIC ? sensors.getTempCByIndex(0) : sensors.getTempFByIndex(0)) * 10.)) / 10.;
 
    if (temperature != 85.00 && temperature > -32.0) {
      char stateTopic[64];
      getStateTopic(stateTopic, sizeof(stateTopic));

      hMqtt.publish(stateTopic, String(temperature).c_str());
    }
  } else {
    delay(millisUntilNextSample);
  }
}

void onConnect() {
  char topic[64];
  char msg[256];
  
  getConfigTopic(topic, sizeof(topic));
  getConfigMessage(msg, sizeof(msg));
  hMqtt.publish(topic, msg, true);
  
  getAvailabilityTopic(topic, sizeof(topic));
  hMqtt.publish(topic, "online", true);
}

int getAvailabilityTopic(char* dest, int n) {
  getAvailabilityTopic(dest, n, SENSOR_NAME);
}

int getConfigTopic(char* dest, int n) {
  return getSensorConfigTopic(dest, n, SENSOR_NAME);
}

int getConfigMessage(char* dest, int n) {
  return getSensorConfigMessage(dest, n, SENSOR_DESCRIPTION, SENSOR_NAME, SENSOR_NAME, "\"unit_of_measurement\": \"°F\"");
}

int getStateTopic(char* dest, int n) {
    return getStateTopic(dest, n, SENSOR_NAME);
}
