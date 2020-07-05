#include <MySensor.h>

#define SWITCH_PIN 8

#define SWITCH_SENSOR_ID 0

MySensor gw;
MyMessage msg(0, 0);

long lastCommandReceived = 0;

void setup() {
    pinMode(SWITCH_PIN, OUTPUT);
    setSwitch(0);

    gw.begin(incomingMessage, AUTO, true);

    // Send the sketch version information to the gateway and Controller
    gw.sendSketchInfo("Furnace Controller", "1.0");

    gw.present(SWITCH_SENSOR_ID, S_BINARY);
}

void loop() {
    gw.process();
    
    long now = millis();
    if (now > (lastCommandReceived + 180000)) {
        setSwitch(0);
    }
}

void incomingMessage(const MyMessage &message) {
    if (message.destination == gw.getNodeId() &&
            message.sensor == SWITCH_SENSOR_ID &&
            mGetCommand(message) == C_SET &&
            message.type == V_STATUS) {
        lastCommandReceived = millis();
        setSwitch(message.getInt());
    }
}

void setSwitch(int status) {
    if (status == 1) {
        digitalWrite(SWITCH_PIN, HIGH);
    } else {
        digitalWrite(SWITCH_PIN, LOW);
    }
}