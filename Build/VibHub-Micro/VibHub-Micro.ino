/* Libraries

External libraries needed:
~~Use https://github.com/JasXSL/WiFiManager - It has a custom ajax endpoint
~~https://arduinojson.org/ - Installable via Arduino IDE.
Ticker-esp32 ??? Don't know where you got this
~~Kadah's fork of arduinoWebSockets: https://github.com/Kadah/arduinoWebSockets
and socket.io-client: https://github.com/JasXSL/socket.io-client
*/ 
#include <Arduino.h>
SET_LOOP_TASK_STACK_SIZE(1024 * 16);

#include "VhWifi.h"

#include <WiFi.h>
#include <HTTPClient.h> 

#include "Configuration.h"
#include "UserSettings.h"
#include "ConfigButton.h"
#include "ApiClient.h"
#include "StatusLED.h"
#include "BatteryReader.h"
#include "VhSerial.h"
#include "esp32s3/rom/rtc.h"
void print_reset_reason(int reason) {
  switch (reason) {
    case 1:  Serial.println("POWERON_RESET"); break;          /**<1,  Vbat power on reset*/
    case 3:  Serial.println("SW_RESET"); break;               /**<3,  Software reset digital core*/
    case 4:  Serial.println("OWDT_RESET"); break;             /**<4,  Legacy watch dog reset digital core*/
    case 5:  Serial.println("DEEPSLEEP_RESET"); break;        /**<5,  Deep Sleep reset digital core*/
    case 6:  Serial.println("SDIO_RESET"); break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7:  Serial.println("TG0WDT_SYS_RESET"); break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8:  Serial.println("TG1WDT_SYS_RESET"); break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9:  Serial.println("RTCWDT_SYS_RESET"); break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10: Serial.println("INTRUSION_RESET"); break;        /**<10, Instrusion tested to reset CPU*/
    case 11: Serial.println("TGWDT_CPU_RESET"); break;        /**<11, Time Group reset CPU*/
    case 12: Serial.println("SW_CPU_RESET"); break;           /**<12, Software reset CPU*/
    case 13: Serial.println("RTCWDT_CPU_RESET"); break;       /**<13, RTC Watch dog Reset CPU*/
    case 14: Serial.println("EXT_CPU_RESET"); break;          /**<14, for APP CPU, reset by PRO CPU*/
    case 15: Serial.println("RTCWDT_BROWN_OUT_RESET"); break; /**<15, Reset when the vdd voltage is not stable*/
    case 16: Serial.println("RTCWDT_RTC_RESET"); break;       /**<16, RTC Watch dog reset digital core and rtc module*/
    default: Serial.println("NO_MEAN");
  }
}

// Program begins
void setup() {
    
    Serial.begin(115200);
    uint8_t n = 0;
    while(!Serial && n < 10) {
        delay(100);
        ++n;
    };
    statusLED.initialize();

    Serial.println("CPU0 reset reason:");
    print_reset_reason(rtc_get_reset_reason(0));

    Serial.println("CPU1 reset reason:");
    print_reset_reason(rtc_get_reset_reason(1));
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

        statusLED.triggerResetWarning();
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
    vhSerial.loop();
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
