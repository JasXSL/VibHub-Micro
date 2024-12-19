#ifndef ChargeDetect_h
#define ChargeDetect_h
#include <Arduino.h>
#include "Configuration.h"

class ChargeDetect{
	public:
		ChargeDetect();
		void loop();
		void setup();
	private:
		uint32_t lastCheck = 0;
};

extern ChargeDetect chargeDetect;


#endif
