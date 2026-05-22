#include "TemperatureReader.h"
#include "Configuration.h"
#include "StatusLED.h"

TemperatureReader::TemperatureReader() {
	temperatureC = 0;
	lastRead = 0;
	supported = false;
	overheating = false;
}

void TemperatureReader::setup() {
	
	pinMode(Configuration::PIN_TEMP_SENSOR, INPUT_PULLDOWN);
	analogRead(Configuration::PIN_TEMP_SENSOR);
	delay(10);
	const uint32_t r = analogReadMilliVolts(Configuration::PIN_TEMP_SENSOR);

	if( r > 400 ){ // Basic feature detect. Going below 400 would mean -7C or below. At which point you probably don't want to use a Lithium battery anyhow.
		supported = true;
		Serial.printf("[INIT] Thermistor OK (%i mV)!\n", r);
	}
	else
		Serial.printf("!! [INIT] Thermistor NOT detected, turning off feature (%i mV). \n", r);

	// Test if feature is supported
	if( !supported ) 
		return;

}

void TemperatureReader::onOverheat() {
	
	overheating = true;
	// To quote the president of madagascar: SHUT. DOWN. EVERYTHING!
	statusLED.setColor(0xFF0000);
	Serial.printf("Shutting down due to overheating (%f C).", temperatureC);
	delay(500);
	esp_sleep_enable_timer_wakeup(5 * 1000000);		// Turn off for 5 sec to prevent overheating
	esp_deep_sleep_start();

}

void TemperatureReader::loop() {

	if( !supported )
		return;

	const uint32_t ms = millis();
	if( ms - lastRead < REFRESH_RATE )
		return;
	lastRead = ms;

	analogRead(Configuration::PIN_TEMP_SENSOR);

	uint32_t highest = 0;
	for( uint8_t i = 0; i < 10; ++i ){

		const uint32_t reading = analogReadMilliVolts(Configuration::PIN_TEMP_SENSOR);
		if( reading > highest )
			highest = reading;
		delayMicroseconds(100);

	}

	const float B_VALUE = 3950; //3450;

	float ratio = (3300 / (float)highest) - 1.0f;
	float inv_t = (1.0f / 298.15) + (1.0f / B_VALUE) * logf(ratio);

	temperatureC = 1.0f / inv_t - 273.15f;
	
	if( temperatureC > OVERHEAT_TEMPERATURE )
		onOverheat();

}


TemperatureReader temperatureReader;
