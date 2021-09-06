
#include <ESP8266WiFi.h>

#include "HWiFi.h"

HWiFi::HWiFi(char* ssid, char* pwd, char* hostname) {
    snprintf(_ssid, sizeof(_ssid), ssid);
    snprintf(_pwd, sizeof(_pwd), pwd);
    snprintf(_hostname, sizeof(_hostname), hostname);
}
    
void HWiFi::start() {
    delay(50);
        
    // Ensure that ESP8266 only starts up in Station mode
    WiFi.mode(WIFI_STA);
        
    // Set the hostname, reusing the mqtt client id
    WiFi.hostname(_hostname);
        
    WiFi.begin(_ssid, _pwd);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}
