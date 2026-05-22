#include "Configuration.h"
#include "BatteryReader.h"
#include "StatusLED.h"
#include "ApiClient.h"
#include <Arduino.h>

BatteryReader::BatteryReader(){
}

void BatteryReader::setup(){
	pinMode(Configuration::PIN_BATTERY_LV, INPUT);
	read();

	const uint32_t mv = getMv();
	if( mv > 3000 && mv < 4300 )
		Serial.printf("[INIT] Battery reader OK (%i mV)\n", mv);
	else
		Serial.printf("!! [INIT] Battery reader FAIL (%i mV)\n", mv);

}

void BatteryReader::read(){

	const uint32_t ms = millis();
	const bool force = !lastRead;
	lastRead = ms;
	_mv = analogReadMilliVolts(Configuration::PIN_BATTERY_LV) * 2; // We're using a 1:1 voltage divider, so multiplying by 2 should give battery voltage
	uint16_t lbVoltage = Configuration::LOW_BATTERY_VOLTAGE;
	if( _isLow )
		lbVoltage += 50; // prevents choppiness
	const bool lowBattery = _mv < lbVoltage;
	if( force || lowBattery != batteryReader.isLow() ){ // Only update if it changed or it's the first time we run
		
		_isLow = lowBattery;
		statusLED.setLowBattery( lowBattery );

	}

}

void BatteryReader::loop(){

	const uint32_t ms = millis();

	if( ms - lastRead > 250 )
		read();

	if( ms - lastRefresh > REFRESH_INTERVAL || !lastRefresh ){
		// Need to send to server
		apiClient.handle_gb(NULL, 0);
		lastRefresh = ms;
	}

}

BatteryReader batteryReader = BatteryReader();
