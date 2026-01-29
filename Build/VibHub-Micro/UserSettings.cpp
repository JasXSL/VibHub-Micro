#include "UserSettings.h"
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include "Configuration.h"
#include "ApiClient.h"

UserSettings::UserSettings(void) :
    //server(Configuration::DEFAULT_HOST),
    port(Configuration::PORT),
    initialized(false)
{
    strcpy(server, Configuration::DEFAULT_HOST);
}


void UserSettings::load( bool reset ){

    Serial.println("UserSettings::load");

    if(!SPIFFS.begin(true)){

        Serial.println("SPIFFS Mount Failed. Device may be damaged or incorrectly compiled without SPIFFS");
        return;

    }
    
    Serial.println("mounted file system");
    
    // Check if config exists
    if( SPIFFS.exists(Configuration::SETTINGS_FILE) ){

        // Reset button held
        if( reset )
            SPIFFS.remove(Configuration::SETTINGS_FILE);
        
        // No reset, load the file
        else{

            //file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open(Configuration::SETTINGS_FILE, "r");
            if( configFile ){

                Serial.println("opened config file:");
                char content[512] = ""; // Max 512 bytes config file 
                uint32_t i = 0;
                while( configFile.available() && i < 511){
                    content[i] = configFile.read();
                    ++i;
                }

                Serial.println(content);
                JsonDocument jsonBuffer;
                DeserializationError error = deserializeJson(jsonBuffer, content);
                

                if( !error ){
                    Serial.println("Parsed json:");
                    serializeJson(jsonBuffer, Serial);
                    Serial.println("");

                    strcpy(server, jsonBuffer["server"]);

                    port = jsonBuffer["port"];
                    strcpy(deviceid, jsonBuffer["deviceid"]);


                    initialized = jsonBuffer["initialized"];

                }
                else
                    Serial.println("failed to load json config");

            }
            
        }

    }
    
    // Output the device info into serial
    Serial.printf("DeviceID: %s\n", deviceid);
    Serial.printf("Server: %s\n", server);
	Serial.printf("Port: %i\n", port);

	if( deviceid[0] == '\0' || port == 0 || port > 65535 || server[0] == '\0' ){

		if( deviceid[0] == '\0' ){

			Serial.println("No device ID found, randomizing one");
			generateDeviceId(false, false);

		}

		if( port == 0 || port > 65535 ){
			
			Serial.println("Invalid port, resetting to default");
			port = Configuration::PORT;

		}

		if( server[0] == '\0' ){

			Serial.println("Invalid server, resetting to factory default");
			strcpy(server, Configuration::DEFAULT_HOST);

		}

		save();

	}
    
}

void UserSettings::generateDeviceId( bool secure, bool sav ){

    gen_random(deviceid, secure);
    if( sav )
        save();

}

// Random device ID generator
void UserSettings::gen_random( char *s, bool secure ){
    
    const int len = secure ? 20 : 12;
    const uint8_t charLen = secure ? 62 : 36;
	char alphanum[charLen+1];
    if( secure )
        strcpy(alphanum, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    else
        strcpy(alphanum, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    
    for( int i = 0; i < len; ++i )
        s[i] = alphanum[esp_random() % (charLen - 1)];
    
    s[len] = 0;

}

// Saves to the config file
void UserSettings::save(){

    Serial.println("UserSettings::save");
	JsonDocument jsonBuffer;
	JsonObject json = jsonBuffer.to<JsonObject>();
	json["server"] = server;
	json["port"] = port;
	json["deviceid"] = deviceid;
    json["initialized"] = initialized;

	File configFile = SPIFFS.open(Configuration::SETTINGS_FILE, "w");
	if( !configFile )
		Serial.println("failed to open config file for writing");
	
    Serial.println("Saving config: ");
	serializeJson(json, Serial);
	serializeJson(json, configFile);
	configFile.close();
    Serial.println();
    
}


void UserSettings::reset(){

    Serial.println("UserSettings::reset");

    if( SPIFFS.begin(true) ){

        if( SPIFFS.exists(Configuration::SETTINGS_FILE) )
            SPIFFS.remove(Configuration::SETTINGS_FILE);
        
    }

}



void UserSettings::loop(){

}



UserSettings userSettings = UserSettings();
