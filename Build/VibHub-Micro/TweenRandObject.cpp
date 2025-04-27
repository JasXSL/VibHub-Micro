#include "TweenRandObject.h"

void TweenRandObject::load( JsonVariant data ){
	reset();

	if( data.is<bool>() )
		isFalse = true;

	else if( data.is<int16_t>() )
		min = max = data.as<int16_t>();
	
	else if( data.is<JsonObject>() ){

		JsonObject d = data.as<JsonObject>();
		if( d["min"] )
			min = d["min"];
		if( d["max"] )
			max = d["max"];
		if( d["offset"] )
			offset = d["offset"];
		if( d["multi"] )
			multi = d["multi"];
			
	}
	else
		Serial.println("RandObject is INVALID");

}

int16_t TweenRandObject::getValue( uint8_t inValue ){

	if( isFalse )
		return inValue;
		
	int16_t out = Configuration::espRandBetween(min, max)*multi+offset;
	//Serial.printf("Returning a random value between min %i, max %i, multi %i, offset %i: %i\n", min, max, multi, offset, out);
	return out;
	
}


void TweenRandObject::reset(){
	min = 0;
	max = 0;
	offset = 0;
	multi = 1;
	isFalse = false;
}