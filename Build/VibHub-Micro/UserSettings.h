#pragma once
#include <Arduino.h>
/*
	VibHub config
*/

class UserSettings{

	public:
		UserSettings(void);
	
	void load( bool reset=false );
	void save();
	void reset();
	
	
	char server[64];
	unsigned int port;
	char deviceid[64];
	bool initialized;
	uint32_t status_led_color = 0x00FF00;           // "connected" LED color
	
	void loop();
	void generateDeviceId( bool secure = false, bool save = true );

	private:
	void gen_random( char *s, bool secure = false );
	
};

extern UserSettings userSettings;

