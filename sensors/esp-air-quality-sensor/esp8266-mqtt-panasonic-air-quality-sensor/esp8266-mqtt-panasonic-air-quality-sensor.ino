
#include <HWiFi.h>
#include <HMqtt.h>
#include <HOpenHab.h>

#include <Wire.h>
#include "SparkFun_Particle_Sensor_SN-GCJA5_Arduino_Library.h"

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
 * #define MQTT_PASSWORD "your-mqtt-password
 * #define SENSOR_SUB_NAME "your-sensor-name"
 * #define SENSOR_LOCATION_DESCRIPTION "Your Sensor Description"
 */
#include "MyConfig.h"

#define I2C_SDA_PIN 1 // i2c SDA / TX
#define I2C_SCL_PIN 3 // i2c SCL / RX

#define ONE_WIRE_PIN 2 // Pin where dallas sensor is connected
#define METRIC false

#define AQI_READING_INTERVAL 1000
#define AQI_READINGS_TO_AVERAGE 30

#define MESSAGE_INTERVAL 30000

#define AQI_TYPE "Aqi"
#define AQI_DESC "AQI"

#define TEMP_TYPE "Temperature"
#define TEMP_DESC "Temperature"

SFE_PARTICLE_SENSOR aqiSensor;

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

HWiFi hWifi(WIFI_SSID, WIFI_PWD, MQTT_CLIENT_ID);
HMqtt hMqtt(MQTT_SERVER, MQTT_PORT, true, (MESSAGE_INTERVAL / 1000) * 2,
            MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);

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

  hWifi.start();

  char willTopic[64];
  getAvailabilityTopic(willTopic, sizeof(willTopic));
  hMqtt.setWill(willTopic, 0, true, "offline"); 

  hMqtt.start();
}
 
void loop()
{
  long now = millis();

  if (now > (aqiReadingTaken + AQI_READING_INTERVAL)) {
    aqiReadings[aqiReadingIndex] = calculatePm25Aqi(aqiSensor.getPM2_5());
    aqiReadingIndex = (aqiReadingIndex + 1) % AQI_READINGS_TO_AVERAGE;
    aqiReadingTaken = now;
  }

  if (now > (lastMsgSent + MESSAGE_INTERVAL) && hMqtt.ensureConnected(onConnect)) {

    sensors.requestTemperatures();
    float temperature = static_cast<float>(static_cast<int>(
      (METRIC ? sensors.getTempCByIndex(0) : sensors.getTempFByIndex(0)) * 10.)) / 10.;
    if (temperature != 85.00 && temperature > -32.0) {
      char tempStateTopic[64];
      getTempStateTopic(tempStateTopic, sizeof(tempStateTopic));
      
      hMqtt.publish(tempStateTopic, String(temperature).c_str());
      lastMsgSent = now;
    }

    int aqiAverage = arrayAverage(aqiReadings, AQI_READINGS_TO_AVERAGE);
    if (aqiAverage != -1) {
      char aqiStateTopic[64];
      getAqiStateTopic(aqiStateTopic, sizeof(aqiStateTopic));
      
      hMqtt.publish(aqiStateTopic, String(aqiAverage).c_str());
      lastMsgSent = now;
    }
  }

  long millisUntilNextReading = (aqiReadingTaken + AQI_READING_INTERVAL) - millis();
  if (millisUntilNextReading > 0) {
    delay(millisUntilNextReading);
  }
}

void onConnect() {
  char topic[64];
  char msg[256];
  
  getAvailabilityTopic(topic, sizeof(topic));

  getTempConfigTopic(topic, sizeof(topic));
  getTempConfigMessage(msg, sizeof(msg));
  hMqtt.publish(topic, msg, true);
  
  getAqiConfigTopic(topic, sizeof(topic));
  getAqiConfigMessage(msg, sizeof(msg));
  hMqtt.publish(topic, msg, true);

  getAvailabilityTopic(topic, sizeof(topic));
  hMqtt.publish(topic, "online", true);
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

int getAvailabilityTopic(char* dest, int n) {
  char name[64];
  getAvailabilityName(name, sizeof(name));
  return getAvailabilityTopic(dest, n, name);
}

int getAqiStateTopic(char* dest, int n) {
  return getMyStateTopic(dest, n, AQI_TYPE);
}

int getTempStateTopic(char* dest, int n) {
  return getMyStateTopic(dest, n, TEMP_TYPE);
}

int getMyStateTopic(char* dest, int n, const char* type) {
  char name[64];
  getStateName(name, sizeof(name), type);
  return getStateTopic(dest, n, name);
}

int getAqiConfigTopic(char* dest, int n) {
  return getConfigTopic(dest, n, AQI_TYPE);
}

int getTempConfigTopic(char* dest, int n) {
  return getConfigTopic(dest, n, TEMP_TYPE);
}

int getConfigTopic(char* dest, int n, const char* type) {
  char name[64];
  getStateName(name, sizeof(name), type);
  return getSensorConfigTopic(dest, n, name);
}

int getAqiConfigMessage(char* dest, int n) {
  return getConfigMessage(dest, n, AQI_DESC, AQI_TYPE, "\"unit_of_measurement\": \"PM2.5\"");
}

int getTempConfigMessage(char* dest, int n) {
  return getConfigMessage(dest, n, TEMP_DESC, TEMP_TYPE, "\"unit_of_measurement\": \"°F\"");
}

int getConfigMessage(char* dest, int n, const char* description, const char* type, const char* extra) {

  char availabilityName[64];
  getAvailabilityName(availabilityName, sizeof(availabilityName));

  char stateName[64];
  getStateName(stateName, sizeof(stateName), type);

  char fullDescription[64];
  snprintf(fullDescription, sizeof(fullDescription), "%s %s", SENSOR_LOCATION_DESCRIPTION, description);
  
  getSensorConfigMessage(dest, n, fullDescription, availabilityName, stateName, extra);
}

int getStateName(char* dest, int n, const char* type) {
  return snprintf(dest, n, "%s%s", SENSOR_SUB_NAME, type);
}

int getAvailabilityName(char* dest, int n) {
  return snprintf(dest, n, "%s%s%sCombo", SENSOR_SUB_NAME, AQI_TYPE, TEMP_TYPE);
}
