/*
	
    Handles socket events from the vibhub server
    
*/

#include "ApiClient.h"
#include "Configuration.h"
#include "StatusLED.h"
#include "UserSettings.h"
#include "StatusLED.h"
#include "BatteryReader.h"
#include <ArduinoJson.h>
#include "VhWifi.h"

ApiClient::ApiClient(void) :
    _socket(),
    _connected(false),
    _running(false)
{
    
}


void ApiClient::setup(){

    for( uint8_t i = 0; i < Configuration::NUM_MOTOR_PINS/2; ++i )
	    motors.push_back(Motor(Configuration::PIN_MOTORS[i*2], Configuration::PIN_MOTORS[i*2+1]));

    // Attach event handlers
    // For simplicity, events are always attached regardless
    _socket.on("connect", std::bind(&ApiClient::event_connect, this, _1, _2));
    _socket.on("disconnect", std::bind(&ApiClient::event_disconnect, this, _1, _2));
    _socket.on("vib", std::bind(&ApiClient::event_vib, this, _1, _2));
    _socket.on("p", std::bind(&ApiClient::event_p, this, _1, _2));
    _socket.on("ps", std::bind(&ApiClient::event_ps, this, _1, _2));
    _socket.on("gb", std::bind(&ApiClient::event_gb, this, _1, _2));

	resetMotors();

}

bool ApiClient::motorRunning(){
    uint8_t i;
    for( i = 0; i < Configuration::NUM_MOTOR_PINS/2; ++i ){
        if( motors[i].running() )
            return true;
    }
    return false;
}


void ApiClient::connect(){

    Serial.printf("ApiClient::connect -> %s:%i\n", userSettings.server, userSettings.port);
    _running = true; // Start loop
    
    if( userSettings.port == 443 ){

        Serial.println("Using SSL");
        _socket.beginSSL(userSettings.server, userSettings.port, Configuration::API_URL, Configuration::ROOTCA);
    }
    else{
        _socket.begin(userSettings.server, userSettings.port, Configuration::API_URL);
    }
}

void ApiClient::disconnect(){

    Serial.println("ApiClient::disconnect");
    _connected = false; // we don't need the disconnect event
    _running = false; // Don't run loop (will cause reconnect)
    _socket.disconnect();
    statusLED.setSocketConnected(false);


}

void ApiClient::event_connect( const char * payload, size_t length ){

	resetMotors();
    Serial.println("ApiClient::event_connect");
    _connected = true;

    JsonDocument doc;
    doc["id"] = userSettings.deviceid;
    doc["version"] = Configuration::VH_VERSION;
    doc["hwversion"] = Configuration::VH_HWVERSION;
    doc["numPorts"] = Configuration::NUM_MOTOR_PINS/2;

    JsonObject capabilities = doc["capabilities"];
    for( uint8_t i = 0; i < Configuration::NR_CAPABILITIES; ++i ){
        if( Configuration::CAPABILITIES[i].modified )
            capabilities[Configuration::CAPABILITIES[i].type] = "modified";
        else
            capabilities[Configuration::CAPABILITIES[i].type] = true;
    }

    char output[256]; // Max 256 bytes output from event_connect
    serializeJson(doc, output);
    Serial.printf("Initializing with: %s\n", output);
    _socket.emit("id", output);
    statusLED.setSocketConnected(true);
;
}

// Payload 
void ApiClient::event_gb( const char * payload, size_t length ){

    JsonDocument jsonBuffer;
    DeserializationError error = deserializeJson(jsonBuffer, payload);
    Serial.print("ApiClient::event_gb: ");
    Serial.println(payload);

    if( error ){
        Serial.println("Unable to parse battery event");
        return;
    }


    // Asks us to reply by sending the same task
    JsonDocument output;
    output["id"] = userSettings.deviceid;
    output["low"] = batteryReader.isLow();
    output["mv"] = batteryReader.getMv();
    output["app"] = jsonBuffer["id"];

    char out[256];
    serializeJson(output, out);
    Serial.printf("ApiClient::event_gb repl sb: %s\n", out);
    _socket.emit("sb", out);

}

void ApiClient::event_disconnect( const char * payload, size_t length ){

    if( _connected ){ // Fire only once

        Serial.println("ApiClient::event_disconnect");
        statusLED.setSocketError(true);
        _connected = false;
        
        resetMotors();

    }

}


void ApiClient::event_vib( const char * payload, size_t length ){

    Serial.printf("ApiClient::event_vib: %s\n", payload);

    JsonDocument jsonBuffer;
    DeserializationError error = deserializeJson(jsonBuffer, payload);
    // Payload: [{stages:[...], port:(int)port}]
    //variant.printTo(Serial);

    if( error ){
        Serial.println("Unable to read vib event");
        return;
    }
    Serial.println("Vib received with");
    serializeJson(jsonBuffer, Serial);
    Serial.println();

    JsonDocument js;
    JsonArray arr = js.to<JsonArray>();

    if( jsonBuffer.is<JsonObject>() ){
        arr.add(jsonBuffer);
    }
    else if( jsonBuffer.is<JsonArray>() ){
        for( uint8_t i=0; i<jsonBuffer.size(); ++i ){
            if( jsonBuffer[i].is<JsonObject>() )
                arr.add(jsonBuffer[i]);
        }
    }else{
        Serial.println("Error: Invalid Vib program received");
    }

    // Cycle through all programs
    for( uint8_t i = 0; i < arr.size(); ++i ){

        JsonObject j = arr[i];

        //Serial.printf("Program %i\n", i);
        //serializeJson(j, Serial);
        //Serial.println();

        const uint8_t numMotors = Configuration::NUM_MOTOR_PINS/2;
        bool mo[numMotors] = {true};
        
        if( j["port"] ){

            uint8_t port = j["port"];
            if( port > 0 ){

                for( uint8_t i = 0; i < numMotors; ++i )
                    mo[i] = port&(1<<i);

            }

        }

        int repeats = 0;
        if( j["repeats"] )
            repeats = j["repeats"];

        for( uint8_t n = 0; n < numMotors; ++n ){

            if( mo[n] ){
                motors[n].loadProgram(j["stages"], repeats);
            }

        }

    }
        

}


void ApiClient::event_p( const char * payload, size_t length ){

    const uint8_t nrMotors = Configuration::NUM_MOTOR_PINS/2;
    uint32_t data = strtoul(payload, 0, 16);
    uint8_t vibArray[nrMotors];
    for( uint8_t i = 0; i < nrMotors; ++i )
        vibArray[i] = (data>>(i*8))&0xFF;

    Serial.printf("ApiClient::event_p - 0x%08x\n", data);

    int i;
    for( i = 0; i < nrMotors; ++i )
        setFlatPWM(i, vibArray[i]);

}

void ApiClient::event_ps( const char * payload, size_t length ){

    Serial.printf("ApiClient::event_ps - %s length %i\n", payload, length);

    // length seems to be off by 12 for some reason?
    if( length > 12 )
        length -= 12;

    // Blocks of 4
    for( size_t i = 0; i < length; i += 4 ){

        char temp[5] = {payload[i], payload[i+1], payload[i+2], payload[i+3]};
        uint32_t sub = strtoul(temp, 0, 16);
        uint8_t chan = (sub>>8);
        uint8_t intens = sub;       // Should work since it shaves off anything left of the first 8 bits
        if( chan > Configuration::NUM_MOTOR_PINS/2 )
            continue;
        setFlatPWM(chan, intens);

    }

}


void ApiClient::setFlatPWM( uint8_t motor, uint8_t value = 0 ){
    motors[motor].stopProgram();  // Stop any running program when this is received
    motors[motor].setPWM(value);
}

void ApiClient::resetMotors(){
    // Reset the motors
    for( uint8_t i = 0; i < Configuration::NUM_MOTOR_PINS/2; ++i )
        setFlatPWM(i);

}

void ApiClient::loop() {

    if (_running){

        _socket.loop();

        for( int i=0; i<Configuration::NUM_MOTOR_PINS/2; ++i )
            motors[i].update();

    }

}


ApiClient apiClient = ApiClient();
