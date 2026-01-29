#include "TweenRandObject.h"

void TweenRandObject::load( JsonVariant data ){
	reset();

	if( data.is<JsonObject>() ){
		//Serial.println("Loading as json object");
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
	else if( data.is<bool>() ){
		isFalse = true;
	}

	else if( data.is<const char*>() ){
		min = max = atoi(data.as<const char*>());
		//Serial.println("Loading as const char*");
	}
	else if( data.is<int>()){
		min = max = data.as<int>();
		//Serial.println("Loading as int");
	}
	else if( data.is<float>() ){
		min = max = round(data.as<float>()*255);
		//Serial.println("Loading as float");
	}
	
	else{

		Serial.println("RandObject is INVALID: ");
		serializeJson(data, Serial);
		Serial.println();

	}
	//Serial.printf("Set min max to %i %i\n", min, max);


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
	// These 2 are mainly used when using randobj in repeats with yoyo
	offset = 0;
	multi = 1;
	isFalse = false;
}