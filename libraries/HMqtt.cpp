
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "HMqtt.h"

// This requires caCert to be defined elsewhere, and should contain
// the root ca cert that the MQTT server is using
// const char caCert[] PROGMEM = R"EOF(
// -----BEGIN CERTIFICATE-----
// ......
// -----END CERTIFICATE-----
// )EOF";
#include <CaCert.h>

HMqtt::HMqtt(const char* server, int port, bool tls, long keepalive,
             const char* clientId, const char* username, const char* password) {
    snprintf(_server, sizeof(_server), server);
    _port = port;
    _tls = tls;
    _keepalive = keepalive;

    snprintf(_clientId, sizeof(_clientId), clientId);
    if (username != NULL && password != NULL) {
        snprintf(_username, sizeof(_username), username);
        snprintf(_password, sizeof(_password), password);
    } else {
        snprintf(_username, sizeof(_username), "");
        snprintf(_password, sizeof(_password), "");
    }
}

void HMqtt::setWill(const char* topic, int qos, bool retain, const char* message) {
    snprintf(_willTopic, sizeof(_willTopic), topic);
    _willQos = qos;
    _willRetain = retain;
    snprintf(_willMessage, sizeof(_willMessage), message);
}

void HMqtt::setMessageCallback(std::function<void(const char*, uint8_t*, unsigned int)> messageCallback) {
    _messageCallback = messageCallback;
}
    
void HMqtt::start() {
    if (_tls) {
        // This does not wait, it just starts the process
        configTime(0, 0, "pool.ntp.org");

        // So we keep on checking to see if our time is some
        // large'ish number, indicating that NTP corrected the
        // time forward.
        time_t now = time(nullptr);
        while (now < 8 * 3600 * 2) {
            delay(500);
            now = time(nullptr);
        }
        
        _caCertX509 = new X509List(caCert);

        _wifiClientSecure = new WiFiClientSecure();
        _wifiClientSecure->setTrustAnchors(_caCertX509);
        
        _espClient = new PubSubClient(*_wifiClientSecure);
    } else {
        _wifiClient = new WiFiClient();

        _espClient = new PubSubClient(*_wifiClient);
    }
    
    _espClient->setServer(_server, _port);
    _espClient->setKeepAlive(_keepalive);
    if (_messageCallback != NULL) {
        _espClient->setCallback(_messageCallback);
    }
}

bool HMqtt::ensureConnected(std::function<void()> connectedCallback) {
    if (!_espClient->connected()) {

        if (_tls) {
            // Not entirely sure if time is called by the cert verification,
            // just in case it is not, call it to ensure that we keep up to
            // "date" on the right time.
            time(nullptr);
        }

        bool success;
        if (strlen(_username) > 0 && strlen(_password) > 0) {
            if (strlen(_willTopic) > 0 && strlen(_willMessage) > 0) {
                success = _espClient->connect(_clientId, _username, _password,
                    _willTopic, _willQos, _willRetain, _willMessage);
            } else {
                success = _espClient->connect(_clientId, _username, _password);
            }
        } else {
            if (strlen(_willTopic) > 0 && strlen(_willMessage) > 0) {
                success = _espClient->connect(_clientId, NULL, NULL,
                    _willTopic, _willQos, _willRetain, _willMessage);
            } else {
                success = _espClient->connect(_clientId);
            }
        }

        if (success) {
            connectedCallback();
        } else {
            // (re)connect failure, sleep and try again later
            delay(5000);
        }
    }
    
    if (_espClient->connected()) {
        // connected - process messages, sleep a bit and go around again
        _espClient->loop();

        return true;
    } else {
        return false;
    }
}

void HMqtt::subscribe(const char* topic) {
    _espClient->subscribe(topic);
}

void HMqtt::publish(const char* topic, const char* payload) {
    _espClient->publish(topic, payload);
}

void HMqtt::publish(const char* topic, const char* payload, bool retain) {
    _espClient->publish(topic, payload, retain);
}
