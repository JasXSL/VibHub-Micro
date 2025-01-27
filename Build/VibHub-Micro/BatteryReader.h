#include "Configuration.h"

class BatteryReader{

	public:
		BatteryReader();
		void setup(); 
		void loop();
		bool isLow(){ return _isLow; }
		uint32_t getMv(){ return _mv; }
	private:
		uint32_t lastRead = 0;
		bool _isLow = false;
		uint32_t _mv = 0;

};


extern BatteryReader batteryReader;


