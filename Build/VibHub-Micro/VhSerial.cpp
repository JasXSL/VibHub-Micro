#include "VhSerial.h"
#include "Configuration.h"
#include "ApiClient.h"
#include "UserSettings.h"
#include <ArduinoJson.h>
#include <WiFiManager.h>

class WifiBreakout: public WiFiManager {
	public:
		void dumpNetworks( char * buffer, size_t length ){
			
			WiFi_scanNetworks(true,false);

			JsonDocument doc;
			JsonArray arr = doc.to<JsonArray>();
			for( uint8_t i = 0; i < _numNetworks; ++i ){

				int16_t rssiperc = getRSSIasQuality(WiFi.RSSI(i));
        		bool encrypted = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
				if( WiFi.SSID(i) == "" )
					continue;

				JsonObject obj = arr.add<JsonObject>();
				obj["ssid"] = WiFi.SSID(i);
				obj["rssi"] = WiFi.RSSI(i);
				obj["rssiPerc"] = rssiperc;
				obj["encrypted"] = encrypted;

			}

			serializeJson(doc, buffer, length);

		}

		uint8_t connect( String ssid, String pass ){
			return connectWifi(ssid, pass, true);
		}

};

VhSerial::VhSerial(){};

// Non debug output always starts with !! followed by task, followed by :
void VhSerial::out( const char * task, const char * message ){

	Serial.print("!!");
	Serial.print(task);
	Serial.print(":");
	Serial.println(message);

}

void VhSerial::finish(){
	buffer[bufferIndex] = 0; // Make sure we have a null terminator
	lastRead = 0; // Reset timer
	
	// Set individual ports
	if( strncmp(buffer, "ps:", 3) == 0 )
		apiClient.event_ps(buffer+3, bufferIndex-3);
	// Set all ports
	else if( strncmp(buffer, "p:", 2) == 0 )
		apiClient.event_p(buffer+2, bufferIndex-2);
	// Get battery info
	else if( strncmp(buffer, "gb:", 3) == 0 ){
		char res[256];
		apiClient.handle_gb(NULL, 0, res, 256);
		out("gb", res);
	}
	// Trigger program
	else if( strncmp(buffer, "vib:", 4) == 0 )
		apiClient.event_vib(buffer+4, bufferIndex-4);
	// Get info about the device
	else if( strncmp(buffer, "connect:", 8) == 0 ){
		char res[256];
		apiClient.handle_connect(buffer+8, bufferIndex-8, res, 256);
		out("connect", res);
	}
	// Reset device id
	else if( strncmp(buffer, "resetid:", 8) == 0 ){

		// bufferIndex is pointing towards the null terminator
		const bool secure = bufferIndex > 8 && buffer[8] == 's' ? true : false;
		Serial.println(buffer);
		userSettings.generateDeviceId(secure, true);
		out("resetid", userSettings.deviceid);

		// Reconnect
		apiClient.disconnect();
		apiClient.connect();

	}
	// Update wifi settings
	else if( strncmp(buffer, "wifi:", 5) == 0 ){
		
		char ssid[128] = {0};
		char pass[128] = {0};
		// Read ssid until next : or end of string
		char * ptr = strtok(buffer+5, ":");
		if( ptr != NULL ){
			strlcpy(ssid, ptr, 128);
			ptr = strtok(NULL, ":");
			if( ptr != NULL ){
				strlcpy(pass, ptr, 128);

				WifiBreakout wifiBreakout;
				const uint8_t res = wifiBreakout.connect(ssid, pass);

				if( res == WL_CONNECTED ){
					out("wifi", "OK");
				}
				else{
					char failNum[8] = {0};
					itoa(res, failNum, 10);
					out("wifi", failNum);
				}

			}
		}


	}
	// Get wifi settings
	else if( strncmp(buffer, "gwifi:", 6) == 0 ){
		
		char res[4096];
		WifiBreakout wifiBreakout;
		wifiBreakout.dumpNetworks(res, 4096);
		out("gwifi", res);

	}
	// Get the current WiFI SSID
	else if( strncmp(buffer, "gnet:", 5) == 0 ){
		out("gnet", WiFi.SSID().c_str());
	}

	

	bufferIndex = 0;	// Reset index

};

void VhSerial::loop(){

	const uint32_t ms = millis();

	while( Serial.available() && bufferIndex < 254 ){

		// Starts the timeout
		if( lastRead == 0 )
			lastRead = ms;

		char c = Serial.read();

		if( c == '\n' || c == '\r' ){ // End of line or carriage return. Finish the transaction.
			finish();
			return;
		}

		buffer[bufferIndex++] = c;
		if( bufferIndex == 255 ){
			finish();
			return;
		}

	}
	
	if( lastRead && ms - lastRead > READ_TIMEOUT ){ // Timeout. Finish the transaction.
		finish();
		return;
	}



};


VhSerial vhSerial = VhSerial();


