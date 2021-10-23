#ifndef HOpenHab_h
#define HOpenHab_h

int getAvailabilityTopic(char* dest, int n, const char* name);

int getCommandTopic(char* dest, int n, const char* name);

int getStateTopic(char* dest, int n, const char* name);

int getSensorConfigTopic(char* dest, int n, const char* name);

int getSwitchConfigTopic(char* dest, int n, const char* name);

int getSensorConfigMessage(char* dest, int n,
                           const char* description,
                           const char* availabilityName,
                           const char* stateName,
                           const char* extra);

int getSwitchConfigMessage(char* dest, int n,
                           const char* description,
                           const char* availabilityName,
                           const char* commandName,
                           const char* stateName,
                           const char* extra);

#endif
