#include "ChargeDetect.h"
#include "StatusLED.h"

ChargeDetect::ChargeDetect(){

}

void ChargeDetect::setup(){
	pinMode(Configuration::PIN_CHRG_STAT, INPUT);
	pinMode(Configuration::PIN_CHRG_STDBY, INPUT);
}

void ChargeDetect::loop(){

	const uint32_t ms = millis();
	if( ms-lastCheck < 100 )
		return;

	lastCheck = ms;
	const bool isCharging = !digitalRead(Configuration::PIN_CHRG_STAT);
	const bool chargeComplete = !digitalRead(Configuration::PIN_CHRG_STDBY);

	statusLED.setCharging( isCharging );
	statusLED.setChargeComplete( !isCharging && chargeComplete );


}

ChargeDetect chargeDetect;
