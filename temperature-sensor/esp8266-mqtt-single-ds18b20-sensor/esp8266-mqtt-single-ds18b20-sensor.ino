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
 * #define MQTT_TOPIC "your-mqtt-topic"
 */
#include "MyConfig.h"
 
#define ONE_WIRE_BUS 2 // Pin where dallas sensor is connected 
#define SAMPLE_INTERVAL 30000
#define METRIC false

WiFiClient espClient;
PubSubClient client(espClient);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

long lastMsgSent = 0;

void setup()
{
  startWifi();
 
  client.setServer(MQTT_SERVER, MQTT_PORT);

  sensors.begin();
}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }

  long now = millis();
  long millisUntilNextSample = (lastMsgSent + SAMPLE_INTERVAL) - now;
  if (millisUntilNextSample <= 0) {
    lastMsgSent = now;

    sensors.requestTemperatures();

    float temperature = static_cast<float>(static_cast<int>(
      (METRIC ? sensors.getTempCByIndex(0) : sensors.getTempFByIndex(0)) * 10.)) / 10.;
 
    if (temperature != 85.00 && temperature > -32.0) {
      client.publish(MQTT_TOPIC, String(temperature).c_str());
    }
  } else {
    delay(millisUntilNextSample);
  }
}

void reconnect() {
  while (!client.connected()) {
#ifdef MQTT_USERNAME
    client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
#else
    client.connect(MQTT_CLIENT_ID);
#endif
  }
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
