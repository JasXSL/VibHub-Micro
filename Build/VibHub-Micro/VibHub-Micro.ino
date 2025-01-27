/* Libraries

External libraries needed:
~~Use https://github.com/JasXSL/WiFiManager - It has a custom ajax endpoint
~~https://arduinojson.org/ - Installable via Arduino IDE.
Ticker-esp32 ??? Don't know where you got this
~~Kadah's fork of arduinoWebSockets: https://github.com/Kadah/arduinoWebSockets
and socket.io-client: https://github.com/Kadah/socket.io-client
*/ 

#include <Arduino.h>

#include "VhWifi.h"

#include <WiFi.h>
#include <HTTPClient.h> 

#include "Configuration.h"
#include "UserSettings.h"
#include "ConfigButton.h"
#include "ApiClient.h"
#include "StatusLED.h"
#include "BatteryReader.h"


// Program begins
void setup() {

    
    Serial.begin(115200);
    statusLED.initialize();

    Serial.println("\nStarting...");

    Serial.printf("Board Version: %s\n", Configuration::VH_HWVERSION);
    Serial.printf("Firmware Version: %s\n", Configuration::VH_VERSION);

    pinMode(Configuration::PIN_NFAULT, INPUT_PULLUP);
    pinMode(Configuration::PIN_NSLEEP, OUTPUT);
    digitalWrite(Configuration::PIN_NSLEEP, HIGH); // Enables the motor driver by default.

    configButton.setup();
    apiClient.setup();
    batteryReader.setup();

    // Reset config and wifi if config button is held on boot
    bool reset = false;
    if( configButton.isPressed() ){

        Serial.println("Resetting everything");
        reset = true;

    }

    userSettings.load(reset);

    statusLED.setBooting(false);
    statusLED.setInitializing(true);

    Serial.println("Connecting to wifi");
    Serial.printf("Reset: %i\n", reset || !userSettings.initialized);
    vhWifi.connect(reset || !userSettings.initialized, reset);
    Serial.println("Connected");

    // Set system time
    setClock();

    //Connect to server
    if( vhWifi.connected ){

        apiClient.connect();
        //apiClient.loop();
        //yield();

    }

    statusLED.setInitializing(false);


}


// Main program lööp
void loop() {

	batteryReader.loop();
    apiClient.loop();
    configButton.loop();
    userSettings.loop();
    statusLED.loop();
    //ArduinoOTA.handle();


}

void setClock() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print("Waiting for NTP time sync: ");
    time_t nowSecs = time(nullptr);
    while (nowSecs < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        yield();
        nowSecs = time(nullptr);
    }

    Serial.println();
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);
    Serial.print("Current time: ");
    Serial.println(asctime(&timeinfo));
}
