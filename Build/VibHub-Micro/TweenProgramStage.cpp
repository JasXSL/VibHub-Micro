#include "TweenProgramStage.h"
#include <vector>
#include "TweenEasing.h"
using namespace TweenEasing;


TweenProgramStage::TweenProgramStage(JsonObject s){

	in = TweenRandObject();
	du = TweenRandObject();
	re = TweenRandObject();

	ease = Ease::LINEAR;
	easeType = EaseType::IN;
	yoyo = false;

	//Serial.println("Adding the following stage:");
	//serializeJson(s, Serial);
	//Serial.println();

	char easing[48] = "Linear.In";
	if( s["i"] ){
		//Serial.println("Loading intensity");
		in.load(s["i"]);
	}
	if( s["d"] ){
		//Serial.println("Loading duration");
		du.load(s["d"]);
	}
	if( s["e"] )
		strcpy(easing, s["e"]);
	if( s["r"] ){
		//Serial.println("Loading repeats");
		re.load(s["r"]);
	}
	if( s["y"] ){
		uint8_t y = s["y"];
		yoyo = y;
	}

	// Figure out the easing
	char *token = strtok(easing, ".");
	std::vector<char*> tokens;
	while( token ){

		tokens.push_back(token);
		token = strtok(NULL,".");

	}

	if( tokens.size() >= 2 ){

		// Translate into a const
		if( strcmp(tokens[0], "Quadratic") == 0 )
			ease = Ease::QUAD;
		else if( strcmp(tokens[0], "Cubic") == 0 )
			ease = Ease::CUBIC;
		else if( strcmp(tokens[0], "Quartic") == 0 )
			ease = Ease::QUART;
		else if( strcmp(tokens[0], "Quintic") == 0 )
			ease = Ease::QUINT;
		else if( strcmp(tokens[0], "Sinusoidal") == 0 )
			ease = Ease::SINE;
		else if( strcmp(tokens[0], "Exponential") == 0 )
			ease = Ease::EXPONENTIAL;
		else if( strcmp(tokens[0], "Circular") == 0 )
			ease = Ease::CIRCULAR;
		else if( strcmp(tokens[0], "Elastic") == 0 )
			ease = Ease::ELASTIC;
		else if( strcmp(tokens[0], "Back") == 0 )
			ease = Ease::BACK;
		else if( strcmp(tokens[0], "Bounce") == 0 )
			ease = Ease::BOUNCE;
		else if( strcmp(tokens[0], "None") == 0 )
			ease = Ease::NONE;
		

		if( strcmp(tokens[1], "Out") == 0 )
			easeType = EaseType::OUT;
		else if( strcmp(tokens[1], "InOut") == 0 )
			easeType = EaseType::INOUT;
		
	}
	
}


void TweenProgramStage::reset(){

	intensity = in.getValue(inValue);
	duration = du.getValue(inValue);
	repeats = re.getValue(inValue);
	if( duration < 0 )
		duration = 0;
	if( repeats < 0 )
		repeats = 0;
	if( intensity < 0 )
		intensity = 0;
	
}

uint32_t TweenProgramStage::getDuration(){

	return duration*(repeats+1);

}

float TweenProgramStage::outValue(){
	return yoyo && repeats%2 ? inValue : intensity;
}

float TweenProgramStage::getValueAtDelta( uint32_t internalDelta ){

	if( internalDelta >= getDuration() )
		return outValue();
	
	if( duration == 0 )
		Serial.printf("ERR: InternalDelta less than 0: %i\n", internalDelta);

	float b = inValue;					// Start value
	float c = intensity-b;				// Change
	// Reverse if yoyo and repeat is odd number
	if( (internalDelta/duration)%2 && yoyo ){
		b = intensity;
		c = inValue-b;
	}
	float t = internalDelta%duration;	// Time in MS of the stage
	float d = duration;

	//Serial.printf("Calculate tween value: Start %f, Change %f, Current Time %f, Total Duration %f \n", b, c, t, d);

	switch(ease){
		case Ease::NONE:
			return easeNone(b,c,t,d);		// Constent tween
		case Ease::BACK:
			return easeBack(b,c,t,d,easeType);
		case Ease::SINE:
			return easeSine(b,c,t,d,easeType);
		case Ease::QUAD:
			return easeQuad(b,c,t,d,easeType);
		case Ease::CUBIC:
			return easeCubic(b,c,t,d,easeType);
		case Ease::QUINT:
			return easeQuint(b,c,t,d,easeType);
		case Ease::BOUNCE:
			return easeBounce(b,c,t,d,easeType);
		case Ease::CIRCULAR:
			return easeCircular(b,c,t,d,easeType);
		case Ease::ELASTIC:
			return easeElastic(b,c,t,d,easeType);
		case Ease::EXPONENTIAL:
			return easeExponential(b,c,t,d,easeType);
	}
	// Linear case
	return easeLinear(b,c,t,d);	// Linear formula

}

