#ifndef HWiFi_h
#define HWiFi_h

class HWiFi {
    public:
        HWiFi(char* ssid, char* pwd, char* hostname);

        void start();
    private:
        char _ssid[64];
        char _pwd[64];
        char _hostname[64];
};

#endif
