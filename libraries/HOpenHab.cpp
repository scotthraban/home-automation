
#include <Arduino.h>

#include "HOpenHab.h"

int getAvailabilityTopic(char* dest, int n, const char* name) {
    return snprintf(dest, n, "iot/%s/availability", name);
}

int getCommandTopic(char* dest, int n, const char* name) {
    return snprintf(dest, n, "iot/%s/command", name);
}

int getStateTopic(char* dest, int n, const char* name) {
    return snprintf(dest, n, "iot/%s/state", name);
}

int getSensorConfigTopic(char* dest, int n, const char* name) {
    return snprintf(dest, n, "homeassistant/sensor/%s/config", name);
}

int getSwitchConfigTopic(char* dest, int n, const char* name) {
    return snprintf(dest, n, "homeassistant/switch/%s/config", name);
}

int getSensorConfigMessage(char* dest, int n,
                           const char* description,
                           const char* availabilityName,
                           const char* stateName,
                           const char* extra) {
    return getSwitchConfigMessage(dest, n, description, availabilityName, NULL, stateName, extra);
}

int getSwitchConfigMessage(char* dest, int n,
                           const char* description,
                           const char* availabilityName,
                           const char* commandName,
                           const char* stateName,
                           const char* extra) {
    
    int offset = 0;
    char buffer[64];
    
    offset += snprintf(dest + offset, n - offset, "{\"name\": \"%s\"", description);

    getAvailabilityTopic(buffer, sizeof(buffer), availabilityName);
    offset += snprintf(dest + offset, n - offset, ", \"availability_topic\": \"%s\"", buffer);

    if (commandName != NULL) {
        getCommandTopic(buffer, sizeof(buffer), commandName);
        offset += snprintf(dest + offset, n - offset, ", \"command_topic\": \"%s\"", buffer);
    }
    
    getStateTopic(buffer, sizeof(buffer), stateName);
    offset += snprintf(dest + offset, n - offset, ", \"state_topic\": \"%s\"", buffer);
  
    if (extra != NULL) {
        offset += snprintf(dest + offset, n - offset, ", %s", extra);
    }
    
    offset += snprintf(dest + offset, n - offset, "}");
    
    return offset;
}

