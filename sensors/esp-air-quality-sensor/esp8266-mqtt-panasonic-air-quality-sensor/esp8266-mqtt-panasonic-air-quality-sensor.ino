
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include "SparkFun_Particle.h"

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
 * #define MQTT_TOPIC_AQI "your-mqtt-topic-aqi"
 * #define MQTT_TOPIC_TEMP "your-mqtt-topic-temp"
 */
#include "MyConfig.h"

#define I2C_SDA_PIN 1 // i2c SDA / TX
#define I2C_SCL_PIN 3 // i2c SCL / RX

#define ONE_WIRE_PIN 2 // Pin where dallas sensor is connected
#define METRIC false

#define AQI_READING_INTERVAL 1000
#define AQI_READINGS_TO_AVERAGE 30

#define MESSAGE_INTERVAL 30000

WiFiClient espClient;
PubSubClient client(espClient);

SFE_PARTICLE_SENSOR aqiSensor;

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

long aqiReadingTaken = 0;
int aqiReadings[AQI_READINGS_TO_AVERAGE];
int aqiReadingIndex = 0;

long lastMsgSent = 0;

void setup()
{
  for (int i = 0; i < AQI_READINGS_TO_AVERAGE; i++) {
    aqiReadings[i] = -1;
  }

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  aqiSensor.begin();

  sensors.begin();

  startWifi();
 
  client.setServer(MQTT_SERVER, MQTT_PORT);
}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }

  long now = millis();

  if (now > (aqiReadingTaken + AQI_READING_INTERVAL)) {
    aqiReadings[aqiReadingIndex] = calculatePm25Aqi(aqiSensor.getPM2_5());
    aqiReadingIndex = (aqiReadingIndex + 1) % AQI_READINGS_TO_AVERAGE;
    aqiReadingTaken = now;
  }

  if (now > (lastMsgSent + MESSAGE_INTERVAL)) {

    sensors.requestTemperatures();
    float temperature = static_cast<float>(static_cast<int>(
      (METRIC ? sensors.getTempCByIndex(0) : sensors.getTempFByIndex(0)) * 10.)) / 10.;
    if (temperature != 85.00 && temperature > -32.0) {
      client.publish(MQTT_TOPIC_TEMP, String(temperature).c_str());
      lastMsgSent = now;
    }

    int aqiAverage = arrayAverage(aqiReadings, AQI_READINGS_TO_AVERAGE);
    if (aqiAverage != -1) {
      client.publish(MQTT_TOPIC_AQI, String(aqiAverage).c_str());
      lastMsgSent = now;
    }
  }

  long millisUntilNextReading = (aqiReadingTaken + AQI_READING_INTERVAL) - millis();
  if (millisUntilNextReading > 0) {
    delay(millisUntilNextReading);
  }
}

int arrayAverage(int arr[], int len) {
  int sum = 0;
  int count = 0;
  for (int i = 0; i < len; i++) {
    if (arr[i] != -1) {
      sum += arr[i];
      count++;
    }
  }

  if (count > 0) {
    return sum / count;
  } else {
    return -1;
  }
}

int calculatePm25Aqi(float pm2_5) {
  // https://forum.airnowtech.org/t/the-aqi-equation/169

  float truncated = trunc(pm2_5 * 10.0) / 10.0;

  if (truncated >= 250.5) {
    return calculateAqi(truncated, 250.5, 500.4, 301, 500);
  } else if (truncated >= 150.5) {
    return calculateAqi(truncated, 150.5, 250.4, 201, 300);
  } else if (truncated >= 55.5) {
    return calculateAqi(truncated, 55.5, 150.4, 151, 200);
  } else if (truncated >= 35.5) {
    return calculateAqi(truncated, 35.5, 55.4, 101, 150);
  } else if (truncated >= 12.1) {
    return calculateAqi(truncated, 12.1, 35.4, 51, 100);
  } else {
    return calculateAqi(truncated, 0.0, 12.0, 0, 50);
  }
}

int calculatePm10Aqi(float pm10) {
  // https://forum.airnowtech.org/t/the-aqi-equation/169

  float truncated = trunc(pm10);

  if (truncated >= 425.0) {
    return calculateAqi(truncated, 425.0, 604.0, 301, 500);
  } else if (truncated >= 355.0) {
    return calculateAqi(truncated, 355.0, 424.0, 201, 300);
  } else if (truncated >= 255.0) {
    return calculateAqi(truncated, 255.0, 354.0, 151, 200);
  } else if (truncated >= 155.0) {
    return calculateAqi(truncated, 155.0, 254.0, 101, 150);
  } else if (truncated >= 55.0) {
    return calculateAqi(truncated, 55.0, 154.0, 51, 100);
  } else {
    return calculateAqi(truncated, 0.0, 54.0, 0, 50);
  }
}

int calculateAqi(float concI, float concL, float concH, int aqiL, int aqiH) {
  return (((aqiH - aqiL) / (concH - concL)) * (concI - concL)) + aqiL;
}

void reconnect() {
  while (!client.connected()) {
#ifdef MQTT_USERNAME
    client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
#else
    client.connect(MQTT_CLIENT_ID);
#endif
  }

  client.setKeepAlive(MESSAGE_INTERVAL * 2);
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
