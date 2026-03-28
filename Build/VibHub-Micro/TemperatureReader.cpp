#include "TemperatureReader.h"
#include "Configuration.h"

TemperatureReader::TemperatureReader() {
	temperatureC = 0;
	lastRead = 0;
	supported = false;
}

void TemperatureReader::setup() {
	
	pinMode(Configuration::PIN_TEMP_SENSOR, INPUT); 			// Set as input
	pinMode(Configuration::PIN_TEMP_SENSOR, INPUT_PULLDOWN);	// Enable pulldown

	analogRead(Configuration::PIN_TEMP_SENSOR);
	delay(10);
	const uint16_t r = analogRead(Configuration::PIN_TEMP_SENSOR);

	if( r > 10 ){ // temp voltage overcomes the pulldown
		supported = true;
		Serial.printf("[TemperatureReader] Detected (%i)!", r);
	}
	else
		Serial.println("[TemperatureReader] Not detected, turning off feature.");

	// Test if feature is supported
	if( !supported ) 
		return;

}


void TemperatureReader::loop() {

	if( !supported )
		return;

	const uint32_t ms = millis();
	if( ms - lastRead < REFRESH_RATE )
		return;
	lastRead = ms;

	Serial.print("Temp sensor ");
	Serial.print(Configuration::PIN_TEMP_SENSOR);
	pinMode(Configuration::PIN_TEMP_SENSOR, INPUT); 			// Set as input
	pinMode(Configuration::PIN_TEMP_SENSOR, INPUT_PULLDOWN);	// Enable pulldown

	analogRead(Configuration::PIN_TEMP_SENSOR);

	uint16_t highest = 0;
	for( uint8_t i = 0; i < 10; ++i ){

		const uint16_t reading = analogRead(Configuration::PIN_TEMP_SENSOR);
		if( reading > highest )
			highest = reading;
		delayMicroseconds(100);

	}
	Serial.printf(" Todo: Temp reading %i\n", highest);

}


TemperatureReader temperatureReader;
