/*
    VibHub Led
*/
#ifndef StatusLED_h
#define StatusLED_h
#include <Arduino.h>
#include <functional>
#include <FastLED.h>
#define NUM_LEDS 1

using namespace std::placeholders;
class StatusLED{

    public:
        StatusLED(void);
        void initialize(); 
        void loop();
        
        void setBooting( bool booting ){ this->booting = booting; loop(); };
        void setInitializing( bool initializing ){ this->initializing = initializing; loop(); };
        void setPortal( bool portal ){ this->portal = portal; Serial.printf("Portal: %d\n", portal); loop(); };
        void setWifiError( bool wifiError ){ this->wifiError = wifiError; loop(); };
        void setSocketError( bool socketError ){ this->socketError = socketError; loop(); };
        //void setCharging( bool charging ){ this->charging = charging; loop(); };
        void setSocketConnected( bool socketConnected ){ this->socketConnected = socketConnected; loop(); };
        void setLowBattery( bool lowBattery ){ this->lowBattery = lowBattery; loop(); };

    private:
        uint8_t programState;           // State the program is currently in
        CRGB leds[NUM_LEDS];
        bool booting = false;           // Base tasks before trying to connect, used to debug
        bool initializing = false;      // We're trying to connect
        bool portal = false;            // We're in config mode
        bool wifiError = false;         // WiFi error found
        bool socketError = false;       // Socket error found
        bool socketConnected = false;   // Everything is connected and setup
        bool lowBattery = false;        
        uint8_t curR = 0;
        uint8_t curG = 0;
        uint8_t curB = 0;
};


extern StatusLED statusLED;

#endif //StatusLED_h