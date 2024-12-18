#include "ChargeDetect.h"
#include "StatusLED.h"

ChargeDetect::ChargeDetect(){}

void ChargeDetect::loop(){

	const uint32_t ms = millis();
	if( ms-lastCheck < 100 )
		return;

	lastCheck = ms;
	const bool isCharging = digitalRead(Configuration::PIN_CHRG_STAT) == LOW;
	const bool chargeComplete = digitalRead(Configuration::PIN_CHRG_STDBY) == LOW;

	statusLED.setCharging( isCharging );
	statusLED.setChargeComplete( !isCharging && chargeComplete );


}
