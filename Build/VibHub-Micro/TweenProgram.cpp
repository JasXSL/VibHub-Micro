#include "TweenProgram.h"
#include "TweenProgramStage.h"
#include <FS.h>

//#define DEBUG

// Constructor
TweenProgram::TweenProgram() :
	repeats(0),
	completed(true),
	highRes(false)
{}
TweenProgram::TweenProgram( int16_t re ){
	reset(re);
}
		
void TweenProgram::addStageFromJson( JsonObject st ){
	
	#ifdef DEBUG 
	Serial.println("Adding a stage");
	#endif
	stages.push_back(
		std::unique_ptr<TweenProgramStage>(new TweenProgramStage(st))
	);

}


// Start the tween
void TweenProgram::start(){

    // Validate program
    if( stages.empty() ){
        return;
    }

	// Reset stuff
	completed = false;
	_repeats = repeats;
	generateStages();

}

// Reshuffles the program stages
void TweenProgram::generateStages(){

	int32_t duration = 0;
	stages[0]->inValue = value;
	for( uint16_t i = 0; i < stages.size(); ++i ){
		
		auto stage = stages[i].get();
		if( i )
			stage->inValue = stages[i-1]->outValue();
		stage->reset();
		#ifdef DEBUG
		Serial.printf("Added stage with intens %i, dur %i \n", stage->intensity, stage->duration);
		#endif
		duration += stage->getDuration();

	}

	_totalTime = duration;
	_started = millis();
	#ifdef DEBUG
	Serial.printf("%i Program stages generated, free heap: %i \n", stages.size(), ESP.getFreeHeap());
	Serial.printf("Program total duration %i, started %i \n", _totalTime, _started);
	#endif
}

// Wipes the stages
void TweenProgram::reset( int16_t rep ){
	
	repeats = _repeats = rep;
	std::vector<std::unique_ptr<TweenProgramStage>>().swap(stages);	// Overwrites the stages

}

// Updates TweenProgram.value Returns false if the program has completed and no stages are left.
bool TweenProgram::loop(){
	
	bool pre = completed;
	if( !completed ){

		const uint32_t delta = millis()-_started;
		// Program has ended
		//Serial.printf("Tick %i %i\n", delta, _totalTime);

		if( delta >= _totalTime ){

			value = stages.back().get()->outValue();	// makes sure the new inValue is proper
			// There are repeats left
			if( repeats == -1 || _repeats ){

				// Reset the stages
				generateStages();

				// Remove from repeat ticker unless infinite
				if( repeats != -1 )
					--_repeats;

			}
			else{

				//Serial.println("DONE");
				completed = true;

			}

		}

		
		// Iterate the stages and find the active one
		uint32_t tot = 0;			// Holds total time for stages, Tracks which program stage we're on
		// Value the tween is starting from
		for( uint16_t i = 0; i < stages.size(); ++i ){
			
			auto stage = stages[i].get();
			const int32_t dur = stage->getDuration();
			// The active stage or if it's the last element
			if( tot+dur >= delta || i == stages.size()-1 ){

				if( (int32_t)delta-tot < 0 ){
					Serial.printf("Invalid delta generated. Delta: %i\n", delta);
				}

				value = stage->getValueAtDelta(delta-tot);	// Delta here is relative to the program
				//Serial.printf("_totalTime %i, _started %i, dur %i, tot %i, delta %i, value %f\n",_totalTime, _started, dur, tot, delta, value);
				break;

			}
			tot+= dur;

		}

	}

	return !pre;

}			
