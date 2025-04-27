#include "Configuration.h"
#include "BatteryReader.h"
#include "StatusLED.h"
#include <Arduino.h>

BatteryReader::BatteryReader(){
}

void BatteryReader::setup(){
	pinMode(Configuration::PIN_BATTERY_LV, INPUT);
	analogSetAttenuation(ADC_11db);
}

void BatteryReader::loop(){

	const uint32_t ms = millis();

	if( ms - lastRead > 250 ){

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

}

BatteryReader batteryReader = BatteryReader();
