
/*
	VibHub motor data
*/
#ifndef Motor_h
#define Motor_h
#include <vector>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include "TweenProgram.h"
// Motor class extending PWM class
class Motor{

	public:
		Motor( uint8_t pin_en, uint8_t pin_rev );								// Creates the motor
		void loadProgram( JsonArray stages, int16_t repeats = 0, bool highRes = false );						// Loads a program onto this
		void update();															// Program loop
		void setPWM( uint16_t duty, bool highRes = false );
		void stopProgram();														// Sets the program as completed
		TweenProgram program;
		bool running();															// Returns true if the duty is > 0

    protected:
        uint16_t _duty;															// Duty cycle (0-255)
		uint8_t pin_en;
		uint8_t pin_rev;

};

#endif
