/*
	Handles serial inputs
*/
#ifndef VhSerial_h
#define VhSerial_h
#include <Arduino.h>


class VhSerial{
	public:
		VhSerial();
		void loop();
		void out( char * task, char * message );
	private:
		uint32_t lastRead = 0;
		const uint32_t READ_TIMEOUT = 10; // ms before forcing end of transaction. We can also send a newline character.
		char buffer[256] = {0}; // Max 256 buffer (last being null terminator)
		uint8_t bufferIndex = 0;

		void finish();

};

extern VhSerial vhSerial;


#endif
