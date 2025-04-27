/*
Driver for one channel of a DRV8833
http://www.ti.com/lit/ds/symlink/drv8833.pdf
*/

#include "Arduino.h"
#include "Motor.h"
#include <ArduinoJson.h> 

//#define DEBUG

Motor::Motor( uint8_t pin_en, uint8_t pin_rev ) :
    _duty(0)
{

	// We're only really using pin_en at the moment. Fast reverse etc could be added later.
	pinMode(pin_rev, OUTPUT);
	digitalWrite(pin_rev, LOW);

	this->pin_en = pin_en;
	this->pin_rev = pin_rev;

	Serial.printf("Setting %i to LOW\n", pin_rev);

	ledcAttach(pin_en, Configuration::PWM_FREQ, Configuration::PWM_RESOLUTION);
	setPWM(0);

}

void Motor::loadProgram( JsonArray stages, int repeats = 0 ){

	#ifdef DEBUG
		Serial.println();
		Serial.printf("Loading new program with #%i stages on motor.\n", stages.size());
	#endif
	
    program.completed = true;
	program.reset(repeats);


	for( auto stage : stages )
		program.addStageFromJson(stage);
	
    Serial.println("Calling program start");
	program.start();

}

void Motor::stopProgram(){
	program.completed = true;	// Makes sure we don't run loop on the program
	program.reset(0);			// Frees up some memory
}

void Motor::update(){

	if( !program.loop() )
		return;

	setPWM((uint8_t)program.value);

}

bool Motor::running(){
    return _duty > 0;
}

void Motor::setPWM( uint8_t duty ){

	if( duty == _duty )
		return;

    #ifdef DEBUG
        Serial.printf("Setting duty: %i on pin %i\n", duty, pin_en);
    #endif
    
	_duty = duty;
	ledcWrite(pin_en, _duty);

}

