#include "StatusLED.h"
#include "Configuration.h"
#include <Arduino.h>
#include "VhWifi.h"
#include <FastLed.h>


StatusLED::StatusLED(){}


void StatusLED::initialize(){

	const uint8_t pin = Configuration::PIN_SLED;
	FastLED.addLeds<WS2812, pin, GRB>(leds, 1);
	booting = true;
	loop();

}


void StatusLED::loop(){

	uint8_t r = 0, g = 50, b = 0;
	const uint32_t ms = millis();
	// Prioritize

	// Booting
	if( booting ){
		r = 1; g = 10;
	}
	// Initializing
	else if( initializing ){
		g = 10;
	}
	// WiFi config mode
	else if( portal ){
		b = 50;
	}
	// Flash blue/red for wifi error
	else if( wifiError ){
		b = 10;
		if( ms%500 < 250 ){
			b = 0;
			r = 10;
		}
	}
	// Flash red green for socket error
	else if( socketError ){
		g = 10;
		if( ms%500 < 250 ){
			g = 0;
			r = 10;
		}
	}
	else if( chargeComplete ){
		// Todo: Fade in and out green
		uint16_t step = ms%1000;
		if( step < 500 )
			g = map(step, 0, 500, 0, 50);
		else
			g = map(step, 500, 1000, 50, 0);
	}
	else if( charging ){
		// Todo: Fade in and out yellow
		uint16_t step = ms%1000;
		if( step < 500 )
			r = map(step, 0, 500, 0, 50);
		else
			r = map(step, 500, 1000, 50, 0);
		g = r;
	}
	else if( !socketConnected ){
		// Blink red on/off
		uint16_t step = ms%1000;
		if( step < 500 )
			r = 10;
	}


	if( r != curR || g != curG || b != curB ){
		curR = r; curG = g; curB = b;
		leds[0] = CRGB(curR, curG, curB);
		FastLED.show();
	}
	

}



StatusLED statusLED = StatusLED();
