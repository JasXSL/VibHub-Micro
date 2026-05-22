#include "StatusLED.h"
#include "Configuration.h"
#include "UserSettings.h"
#include <Arduino.h>
#include "VhWifi.h"



StatusLED::StatusLED() : pixels(std::make_unique<LiteLED>(LiteLED(LED_STRIP_WS2812, 0))){

}


void StatusLED::initialize(){
	pixels->begin(Configuration::PIN_SLED, NUM_LEDS);
	booting = true;
	loop();

}

// Blink 10 times rapidly to alert that we're factory resetting the device
void StatusLED::triggerResetWarning(){
	for( uint8_t i = 0; i < 10; i++ ){
		pixels->setPixel(0, 255<<16);
		pixels->show();
		delay(50);
		pixels->setPixel(0, 0);
		pixels->show();
		delay(50);
	}
}


void StatusLED::loop(){

	uint8_t r = 0, g = 0, b = 0;
	const uint32_t ms = millis();
	// Prioritize

	// [Yellowish green] Booting
	if( booting ){
		r = 1; g = 10;
	}
	// [blue red flash] for wifi error
	else if( wifiError ){
		b = 10;
		if( ms%500 < 250 ){
			b = 0;
			r = 10;
		}
	}
	// [red green flash] for socket error
	else if( socketError ){
		g = 10;
		if( ms%500 < 250 ){
			g = 0;
			r = 10;
		}
	}

	// [Blue] WiFi config mode
	else if( portal ){
		b = 50;
	}
	
	// [Green cyan] Initializing
	else if( initializing ){
		g = 10; b = 5;
	}
	/*
	// Legacy
	// [Green fade in and out] charging is complete, and not connected to a computer
	else if( chargeComplete && !usb_serial_jtag_is_connected() ){
		
		uint16_t step = ms%3000;
		if( step < 1500 )
			g = map(step, 0, 1500, 0, 50);
		else
			g = map(step, 1500, 3000, 50, 0);
			
	}
	// [Yellow fade in and out] charging
	else if( charging && !usb_serial_jtag_is_connected() ){

		uint16_t step = ms%3000;
		if( step < 1500 )
			r = map(step, 0, 1500, 0, 50);
		else
			r = map(step, 1500, 3000, 50, 0);
		g = r;
		
	}
	*/
	// Blink red on/off while not connected
	else if( !socketConnected ){
		uint16_t step = ms%1000;
		if( step < 500 )
			r = 10;
	}
	// Normal operation
	else{
		
		r = (userSettings.status_led_color>>16) & 0xFF;
		g = (userSettings.status_led_color>>8) & 0xFF;
		b = userSettings.status_led_color & 0xFF;

		// Double pink flash for app connected
		const uint32_t delta = ms-lastAppConnect;
		if( delta < 500 && lastAppConnect ){

			if( delta%250 < 125 )
				r = b = 255;
			
		}

		// Blink yellow while low battery.
		else if( lowBattery && ms%2000 < 1000 ){
			r = g = 255;
			b = 0;
		}
		

	}

	if( r != curR || g != curG || b != curB ){
		//Serial.printf("New color: %d %d %d\n", r, g, b);
		curR = r; curG = g; curB = b;
		pixels->setPixel(0, (r<<16)|(g<<8)|b);
		pixels->show();
	}
	

}

// Overrides color. Usually followed by a sleep or a loop. Overridden next time StatusLED::loop is called
void StatusLED::setColor( uint32_t color ){
	Serial.printf("R %i, G %i, B %i\n", (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
	pixels->setPixel(0, color);
	pixels->show();
}


StatusLED statusLED = StatusLED();
