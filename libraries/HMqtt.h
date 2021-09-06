#ifndef HMqtt_h
#define HMqtt_h

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class HMqtt {
    public:
        HMqtt(const char* server, int port, bool tls, long keepalive,
              const char* clientId, const char* username, const char* password);

        void setWill(const char* topic, int qos, bool retain, const char* message);
        void setMessageCallback(std::function<void(const char*, uint8_t*, unsigned int)> messageCallback);

        void start();
        bool ensureConnected(std::function<void()> connectedCallback);

        void subscribe(const char* topic);

        void publish(const char* topic, const char* message);
        void publish(const char* topic, const char* message, bool retain);

    private:
        char _server[64];
        int _port;
        boolean _tls;
        long _keepalive;
        
        char _clientId[64];
        char _username[64];
        char _password[64];
        
        char _willTopic[64];
        int _willQos;
        bool _willRetain;
        char _willMessage[256];

        std::function<void(const char*, uint8_t*, unsigned int)> _messageCallback;
        
        X509List* _caCertX509;
        WiFiClientSecure* _wifiClientSecure;
        WiFiClient* _wifiClient;
        PubSubClient* _espClient;
};

#endif
