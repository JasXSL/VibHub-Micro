#ifndef TweenProgram_h
#define TweenProgram_h
#include <vector>
#include "TweenProgramStage.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <FS.h>

class TweenProgram{
	public:
		std::vector<std::unique_ptr<TweenProgramStage>> stages;
		int16_t repeats;			// Total repeats for this program
		float value;			// Current tween value
		bool completed;			// Program completed

		TweenProgram();
		TweenProgram( int16_t re );
		void addStageFromJson(JsonObject st);
		void start();								// Resets loops, generates randobjects, and starts the tween
		void generateStages();						// Unsed on program start or loop to reload the stages with randobjects etc
		void reset( int16_t rep );						// Resets the stages and sets repeats, lets you keep the program without destroying it
		bool loop();			// Updates TweenProgram.value Returns false if the program has completed and no stages are left.
		

	private:
		uint32_t _started;			// Time the program started
		uint32_t _totalTime; 		// Total time for one iteration of the program
		int16_t _repeats;			// Repeats left until the program ends
};

#endif