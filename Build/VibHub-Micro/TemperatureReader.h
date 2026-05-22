#pragma once
#include "Configuration.h"

class TemperatureReader{

	public:
		
		static const uint32_t REFRESH_INTERVAL = 30e3; // Send to server every 30 seconds
		TemperatureReader();
		void setup(); 
		void loop();
		void onOverheat();
		float temperatureC;
		bool overheating;

	private:
		static constexpr float OVERHEAT_TEMPERATURE = 55.0f;
		static const uint32_t REFRESH_RATE = 1000;
		uint32_t lastRead;
		bool supported;

};


extern TemperatureReader temperatureReader;


