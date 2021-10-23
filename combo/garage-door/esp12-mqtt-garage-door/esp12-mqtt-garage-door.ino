
#include <HWiFi.h>
#include <HMqtt.h>
#include <HOpenHab.h>

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
 * #define MQTT_TLS if using TLS - note the cert config is required as well
 * #define SENSOR_NAME "your-sensor-name"
 * #define SENSOR_LOCATION_DESCRIPTION "Your Sensor Description"
 */
#include "MyConfig.h"
 
#define TRANSISTOR_OUT_PIN  D1  // Pin where transistor output is connected, high to activate relay
#define REED_SWITCH_IN_PIN  D2  // Pin where reed switch is connected - high is door closed

#define NUM_READ_STATES     10
#define SEND_STATE_REFRESH_INTERVAL 20000 // Garage door takes about 12 seconds to close

#define DOOR_OPEN           LOW
#define DOOR_CLOSED         HIGH

#define DOOR_OPEN_PAYLOAD   "ON"
#define DOOR_CLOSED_PAYLOAD "OFF"

#define LED_ON              LOW
#define LED_OFF             HIGH

int latestStates[NUM_READ_STATES];
int lastSentState = -1;
long lastStateSent = 0;

HWiFi hWifi(WIFI_SSID, WIFI_PWD, MQTT_CLIENT_ID);
HMqtt hMqtt(MQTT_SERVER, MQTT_PORT, true, (SEND_STATE_REFRESH_INTERVAL / 1000) * 2,
            MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TRANSISTOR_OUT_PIN, OUTPUT);

  pinMode(REED_SWITCH_IN_PIN, INPUT);

  digitalWrite(LED_BUILTIN, LED_OFF);
  digitalWrite(TRANSISTOR_OUT_PIN, LOW);

  for (int i = 0; i < NUM_READ_STATES; i++) {
    latestStates[i] = -1;
  }

  hWifi.start();

  char willTopic[64];
  getAvailabilityTopic(willTopic, sizeof(willTopic));
  hMqtt.setWill(willTopic, 0, true, "offline"); 

  hMqtt.setMessageCallback(processMessage);
  hMqtt.start();
}

void loop() {
  if (hMqtt.ensureConnected(onConnect)) {
    readState();
    sendState();

    delay(500);
  }
}

void sendState()
{
  int latestState = getLatestState();
  
  if (isStateStable() && (lastSentState != latestState || isTimeToSendStateRefresh())) {
    if (latestState == DOOR_CLOSED) {
      char stateTopic[64];
      getStateTopic(stateTopic, sizeof(stateTopic));

      digitalWrite(LED_BUILTIN, LED_OFF);
      hMqtt.publish(stateTopic, DOOR_CLOSED_PAYLOAD);
    } else if (latestState == DOOR_OPEN) {
      char stateTopic[64];
      getStateTopic(stateTopic, sizeof(stateTopic));

      digitalWrite(LED_BUILTIN, LED_ON);
      hMqtt.publish(stateTopic, DOOR_OPEN_PAYLOAD);
    }

    lastSentState = latestState;
    lastStateSent = millis();
  }
}

bool isTimeToSendStateRefresh()
{
  return millis() > (lastStateSent + SEND_STATE_REFRESH_INTERVAL);  
}

int getLatestState()
{
  return latestStates[NUM_READ_STATES - 1];
}

void readState()
{
  for (int i = 0; i < (NUM_READ_STATES - 1); i++) {
    latestStates[i] = latestStates[i + 1];
  }
  latestStates[NUM_READ_STATES - 1] = digitalRead(REED_SWITCH_IN_PIN);
}

bool isStateStable()
{
  int state = latestStates[0];
  for (int i = 0; i < NUM_READ_STATES; i++) {
    if (state != latestStates[i]) {
      return false;
    }
  }
  return true;
}

void processMessage(const char* topic, byte* payload, unsigned int length)
{
  char commandTopic[64];
  getCommandTopic(commandTopic, sizeof(commandTopic));

  if (strncmp(commandTopic, topic, strlen(commandTopic)) != 0) {
    // this is not the topic you are looking for
    return;
  }

  if (length == strlen(DOOR_OPEN_PAYLOAD) && strncmp(DOOR_OPEN_PAYLOAD, (char*)payload, length) == 0) {
    if (isStateStable() && getLatestState() == DOOR_CLOSED) {
      toggleDoorState(LED_OFF);
    }
  } else if (length == strlen(DOOR_CLOSED_PAYLOAD) && strncmp(DOOR_CLOSED_PAYLOAD, (char*)payload, length) == 0) {
    if (isStateStable() && getLatestState() == DOOR_OPEN) {
      toggleDoorState(LED_ON);
    }
  }
}

void toggleDoorState(int ledFinal)
{
  digitalWrite(TRANSISTOR_OUT_PIN, HIGH);
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(100);
    digitalWrite(LED_BUILTIN, LED_OFF);
    delay(100);
  }
  digitalWrite(TRANSISTOR_OUT_PIN, LOW);
  digitalWrite(LED_BUILTIN, ledFinal);
}

void onConnect() {
  char topic[64];
  char msg[256];

  getCommandTopic(topic, sizeof(topic));
  hMqtt.subscribe(topic);

  getConfigTopic(topic, sizeof(topic));
  getConfigMessage(msg, sizeof(msg));
  hMqtt.publish(topic, msg, true);
  
  getAvailabilityTopic(topic, sizeof(topic));
  hMqtt.publish(topic, "online", true);
}

int getCommandTopic(char* dest, int n) {
  return getCommandTopic(dest, n, SENSOR_NAME);
}

int getStateTopic(char* dest, int n) {
  return getStateTopic(dest, n, SENSOR_NAME);
}

int getAvailabilityTopic(char* dest, int n) {
  return getAvailabilityTopic(dest, n, SENSOR_NAME);
}

int getConfigTopic(char* dest, int n) {
  return getSwitchConfigTopic(dest, n, SENSOR_NAME);
}

int getConfigMessage(char* dest, int n) {
  return getSwitchConfigMessage(dest, n, SENSOR_DESCRIPTION, SENSOR_NAME, SENSOR_NAME, SENSOR_NAME, NULL);
}
