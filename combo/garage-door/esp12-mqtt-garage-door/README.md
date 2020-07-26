# ESP8266 MQTT over Wifi garage door controller / sensor

This project is written to be compiled from the Arduino IDE for the ESP8266 / ESP-12e.

Due to limitations on IO pins that are stable during bootup, this project required the use of an ESP-12e, rather than the ESP-01 that I usually use. Since the relay opens the garage door, I definitely don't want the garage door to be triggered during a reboot or startup.

Credit is due to Stephen Warren, who shared his research on the topic in his Down the Rabbit Hole blog entry [ESP8266 GPIO Behaviour at Boot](https://rabbithole.wwwdotorg.org/2017/03/28/esp8266-gpio.html). His findings were quite helpful in steering me in the right direction for this project.

There are a number of parameters that will be specific to the IOT environment you are installing the controller into.

To configure, put a number of #define entries into a MyConfig.h file in the same directory as the project file:

```
#define WIFI_SSID "Your SSID"
#define WIFI_PWD "Your-Password"
#define MQTT_SERVER "your.server.com"
#define MQTT_PORT your-mqtt-port
#define MQTT_CLIENT_ID "your-client-id"
#define MQTT_USERNAME "your-mqtt-username"
#define MQTT_PWD "your-mqtt-password
#define MQTT_PUBLISH_TOPIC "your-mqtt-publish-topic"
#define MQTT_SUBSCRIBE_TOPIC "your-mqtt-subscribe-topic"
```

Better would be to break up the common MQTT and WiFi params into their own files and put those into you Arduino IDE path, so that you can reuse them, and then you just need to include those two files in the MyConfig.h files, and specify the MQTT client id and topic, since those would be specific to each controller you build.
