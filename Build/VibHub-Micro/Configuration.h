/*
    This is the project configuration, these values don't change
*/
#ifndef _VH_CONFIGURATION
#define _VH_CONFIGURATION
#include <FS.h>
#include <Arduino.h>
// Bug with arduino unless I undefine these
#undef min
#undef max

#define CTASK_PWM_BASIC "p"                     // Capable of handling motor updates in one go. Data contains hex encoded bytes for each motor, with the first motor being the rightmost hex pair. If data contains more pairs than we have motors, the first entry becomes a settings byte.
    const uint8_t CTASK_PWM_BASIC_HIGHRES = 0x1;        // Indicates that we're using highres, and that data comes as 16 bit hex encodings instead of 8, MSB left.
#define CTASK_PWM_SPECIFIC "ps"                 // Capable of handling individual motor updates. Bytes are received as hex, with the first pair being the motor index, and the second pair being the duty cycle. The leftmost bit is reserved as a "high res" flag, in which case the data is received in groups of 3 hex encoded bytes instead of 2.
#define CTASK_PROGRAMS "vib"                    // Capable of handling programs
#define CTASK_APP_OFFLINE "app_offline"         // Capable of doing something when app goes offline
#define CTASK_CUSTOM_TO_DEVICE "dCustom"        // Capable of receiving custom data from the app
#define CTASK_DEVICE_TO_CUSTOM "aCustom"        // Capable of sending custom data to the app
#define CTASK_BATTERY_LEVEL "sb"                // Capable of reporting battery status
#define CTASK_HIGHRES "h"                       // Capable of handling high-res versions of PWM_BASIC/SPECIFIC and PROGRAMS. The desc is a string representation of the max nr of bits we support.              
class Capability{
    public:
        Capability( const char* t, char * d = NULL ){
            type = (char*)t;    
            if( d != NULL )
                strlcpy(desc, d, 16);
        }
        char *type;
        char desc[16] = "";
};


namespace Configuration{

    const char VH_VERSION[]         = "mc0.0.3";          // Software version
    const char VH_HWVERSION[]       = "MicroHub R1";      // Hardware type & version
    const char WIFI_SSID[]          = "VibHub Micro";         // Network SSID
    const char DEFAULT_HOST[]       = "vibhub.io";      // Default socket host
    const uint16_t  PORT            = 80;               // Default socket port
    const char API_URL[]            = "/socket.io/?transport=websocket";
    const char SETTINGS_FILE[]      = "/config.json";   // Default SPIFFS config file

    const uint8_t NR_CAPABILITIES   = 5;
    const Capability CAPABILITIES[NR_CAPABILITIES] = {
        Capability(CTASK_PWM_BASIC),
        Capability(CTASK_PWM_SPECIFIC),
        Capability(CTASK_PROGRAMS),
        Capability(CTASK_BATTERY_LEVEL),
        Capability(CTASK_HIGHRES, "12")
    };

    // Websockets
    const uint16_t WEBSOCKET_RECONNECT_TIME = 3000;          // How long to wait after a connection failed

    // Status LED
    const uint8_t SLED_BRIGHTNESS   = 50;   // Status LED brightness, 0-255

    // ESP32-S3 Board GPIO Assignments
    const uint8_t PIN_SLED        = 38;     // WS2812 Status LED
    
    const uint8_t NUM_MOTOR_PINS = 4;
    const uint8_t PIN_MOTORS[NUM_MOTOR_PINS] = {36,37, 2,1}; // A/B pairs
    const uint8_t PIN_CONFIG_BUTTON = 5;   // Configuration pin. Use internal pullup   

    // Battery level
    const uint8_t PIN_BATTERY_LV = 10;
    const uint32_t LOW_BATTERY_VOLTAGE = 3400; // Millivolts ~3.4V
    const uint32_t MAX_BATTERY_VOLTAGE = 4200;

    // PWM
    const uint16_t PWM_FREQ         = 8000;    // PWM Frequency
    const uint16_t PWM_RESOLUTION   = 12;       // PWM resolution

    // Motor driver nFault
    const uint8_t PIN_NFAULT = 12;
    const uint8_t PIN_NSLEEP = 11;

    // Button config
    const bool BUTTON_UP            = HIGH;  // Value when the button is not pressed
    const bool BUTTON_DOWN          = LOW;   // Value when the button IS pressed
    const uint16_t BUTTON_HOLD_TIME = 4000;  // Time before it enters config mode
    const uint8_t BUTTON_DEBOUNCE   = 100;   // Debounce time

    const bool PWR_BUTTON_UP        = HIGH;  // Power button not pressed
    const bool PWR_BUTTON_DOWN      = LOW;  // Power button pressed
    
    // Global randomizer function
    // Returns a random value which can be min through and including max
    // min0 max3 would generate 0, 1, 2, or 3
    inline int espRandBetween(int minimum, int maximum){
        float r = (float)esp_random()/UINT32_MAX;
        return (minimum+floor((maximum+1-minimum)*r));
    }

    const char ROOTCA[] = \
        "-----BEGIN CERTIFICATE-----"
"MIIFBjCCAu6gAwIBAgIRAIp9PhPWLzDvI4a9KQdrNPgwDQYJKoZIhvcNAQELBQAw"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw"
"WhcNMjcwMzEyMjM1OTU5WjAzMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg"
"RW5jcnlwdDEMMAoGA1UEAxMDUjExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB"
"CgKCAQEAuoe8XBsAOcvKCs3UZxD5ATylTqVhyybKUvsVAbe5KPUoHu0nsyQYOWcJ"
"DAjs4DqwO3cOvfPlOVRBDE6uQdaZdN5R2+97/1i9qLcT9t4x1fJyyXJqC4N0lZxG"
"AGQUmfOx2SLZzaiSqhwmej/+71gFewiVgdtxD4774zEJuwm+UE1fj5F2PVqdnoPy"
"6cRms+EGZkNIGIBloDcYmpuEMpexsr3E+BUAnSeI++JjF5ZsmydnS8TbKF5pwnnw"
"SVzgJFDhxLyhBax7QG0AtMJBP6dYuC/FXJuluwme8f7rsIU5/agK70XEeOtlKsLP"
"Xzze41xNG/cLJyuqC0J3U095ah2H2QIDAQABo4H4MIH1MA4GA1UdDwEB/wQEAwIB"
"hjAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwEgYDVR0TAQH/BAgwBgEB"
"/wIBADAdBgNVHQ4EFgQUxc9GpOr0w8B6bJXELbBeki8m47kwHwYDVR0jBBgwFoAU"
"ebRZ5nu25eQBc4AIiMgaWPbpm24wMgYIKwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAC"
"hhZodHRwOi8veDEuaS5sZW5jci5vcmcvMBMGA1UdIAQMMAowCAYGZ4EMAQIBMCcG"
"A1UdHwQgMB4wHKAaoBiGFmh0dHA6Ly94MS5jLmxlbmNyLm9yZy8wDQYJKoZIhvcN"
"AQELBQADggIBAE7iiV0KAxyQOND1H/lxXPjDj7I3iHpvsCUf7b632IYGjukJhM1y"
"v4Hz/MrPU0jtvfZpQtSlET41yBOykh0FX+ou1Nj4ScOt9ZmWnO8m2OG0JAtIIE38"
"01S0qcYhyOE2G/93ZCkXufBL713qzXnQv5C/viOykNpKqUgxdKlEC+Hi9i2DcaR1"
"e9KUwQUZRhy5j/PEdEglKg3l9dtD4tuTm7kZtB8v32oOjzHTYw+7KdzdZiw/sBtn"
"UfhBPORNuay4pJxmY/WrhSMdzFO2q3Gu3MUBcdo27goYKjL9CTF8j/Zz55yctUoV"
"aneCWs/ajUX+HypkBTA+c8LGDLnWO2NKq0YD/pnARkAnYGPfUDoHR9gVSp/qRx+Z"
"WghiDLZsMwhN1zjtSC0uBWiugF3vTNzYIEFfaPG7Ws3jDrAMMYebQ95JQ+HIBD/R"
"PBuHRTBpqKlyDnkSHDHYPiNX3adPoPAcgdF3H2/W0rmoswMWgTlLn1Wu0mrks7/q"
"pdWfS6PJ1jty80r2VKsM/Dj3YIDfbjXKdaFU5C+8bhfJGqU3taKauuz0wHVGT3eo"
"6FlWkWYtbt4pgdamlwVeZEW+LM7qZEJEsMNPrfC03APKmZsJgpWCDWOKZvkZcvjV"
"uYkQ4omYCTX5ohy+knMjdOmdH9c7SpqEWBDC86fiNex+O0XOMEZSa8DA"
"-----END CERTIFICATE-----";
};



#endif