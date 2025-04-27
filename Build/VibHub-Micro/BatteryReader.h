#include "Configuration.h"

class BatteryReader{

	public:
		static const uint32_t REFRESH_INTERVAL = 30e3; // Send to server every 30 seconds
		BatteryReader();
		void setup(); 
		void loop();
		bool isLow(){ return _isLow; }
		uint32_t getMv(){ return _mv; }
	private:
		uint32_t lastRead = 0;
		uint32_t lastRefresh = 0;	// update
		bool _isLow = false;
		uint32_t _mv = 0;

};


extern BatteryReader batteryReader;


