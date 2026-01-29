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

void ApiClient::handle_connect( const char * payload, size_t length, char * out, size_t outLength ){

    JsonDocument doc;
    doc["id"] = userSettings.deviceid;
    doc["version"] = Configuration::VH_VERSION;
    doc["hwversion"] = Configuration::VH_HWVERSION;
    doc["numPorts"] = Configuration::NUM_MOTOR_PINS/2;

    JsonObject capabilities = doc["capabilities"].to<JsonObject>();
    for( uint8_t i = 0; i < Configuration::NR_CAPABILITIES; ++i ){
        
        const char * desc = Configuration::CAPABILITIES[i].desc;
        if( desc[0] )
            capabilities[Configuration::CAPABILITIES[i].type] = desc;
        else
            capabilities[Configuration::CAPABILITIES[i].type] = true;
    }

    if( out == NULL ){

        // Output to socket
        char output[256]; // Max 256 bytes output from event_connect
        serializeJson(doc, output);
        Serial.printf("Initializing with: %s\n", output);
        _socket.emit("id", output);

    }
    // Output to out var
    else{
        serializeJson(doc, out, outLength);
    }

}

void ApiClient::event_connect( const char * payload, size_t length ){

	resetMotors();
    Serial.println("ApiClient::event_connect");
    _connected = true;

    handle_connect(payload, length);
    statusLED.setSocketConnected(true);
    statusLED.setSocketError(false);

}

// Keep in mind that payload may be NULL
void ApiClient::handle_gb( const char * payload, size_t length, char * out, size_t outLength ){
    JsonDocument jsonBuffer;
    if( payload ){
        DeserializationError error = deserializeJson(jsonBuffer, payload);
        Serial.print("ApiClient::event_gb: ");
        Serial.println(payload);
        if( error ){
            Serial.println("Unable to parse battery event");
            return;
        }
    }

    Serial.printf("Free ram = %i\n", ESP.getFreeHeap());

    // Asks us to reply by sending the same task
    JsonDocument output;
    output["id"] = userSettings.deviceid;
    output["low"] = batteryReader.isLow();
    output["mv"] = batteryReader.getMv();
    output["xv"] = Configuration::MAX_BATTERY_VOLTAGE;
    if( payload )
        output["app"] = jsonBuffer["id"];

    // Raise a response event
    if( out == NULL ){

        char out[256];
        serializeJson(output, out);
        Serial.printf("ApiClient::event_gb repl sb: %s\n", out);
        _socket.emit("sb", out);

    }
    // Just overwrite out
    else{
        serializeJson(output, out, outLength);
    }
}
// Tunnels into handle_gb, which can be used by vhSerial
void ApiClient::event_gb( const char * payload, size_t length ){
    handle_gb(payload, length);
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
    length = strnlen(payload, length);

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
        bool mo[numMotors];
        for( uint8_t n = 0; n < numMotors; ++n )
            mo[n] = true;
        
        if( j["port"] ){

            int16_t port = j["port"];
            if( port > 0 ){

                for( uint8_t i = 0; i < numMotors; ++i )
                    mo[i] = port&(1<<i);

            }

        }

        int repeats = 0;
        if( j["repeats"] )
            repeats = j["repeats"];

        bool highres = false;
        if( j["highres"] )
            highres = j["highres"];

        for( uint8_t n = 0; n < numMotors; ++n ){

            if( mo[n] ){
                motors[n].loadProgram(j["stages"], repeats, highres);
            }

        }

    }
        

}


void ApiClient::event_p( const char * payload, size_t length ){
    length = strnlen(payload, length); // For some reason, length is often wrong here

    const uint8_t nrMotors = Configuration::NUM_MOTOR_PINS/2;
    uint8_t flags = 0;
    bool highRes = false;
    if( length > nrMotors*2 ){

        char first[3] = {0};
        strncpy(first, payload, 2); 
        flags = strtoul(first, 0, 16);
        if( flags & 0x01 )
            highRes = true;

    }
    
    for( uint8_t i = 0; i < nrMotors; ++i ){
        
        const uint8_t offs = 2+highRes*2;
        uint8_t startFrom = length - i*offs - offs;
        char group[3+offs] = {0};
        strncpy(group, payload+startFrom, offs);
        setFlatPWM(i, strtoul(group, 0, 16), highRes); // Assigns 32bit to 16 bit, but that's because there's no unsigned strto function for 16bit

    }

    Serial.printf("ApiClient::event_p - %s, length %i, nrMotors %i, highRes %i\n", payload, length, nrMotors, highRes);       

}

// receiving a hex string
void ApiClient::event_ps( const char * payload, size_t length ){
    length = strnlen(payload, length);

    Serial.printf("ApiClient::event_ps - %s length %i\n", payload, length);
    size_t i = 0;

    while( i < length ){

        char temp[3] = {0};
        strlcpy(temp, payload+i, 3);
        uint16_t targetMotor = strtoul(temp, 0, 16);
        const bool isHighRes = targetMotor & 0b10000000; // Leftmost bit is reserved for high res toggle
        targetMotor = targetMotor &~ 0b10000000; // Remove leftmost bit to get a motor number
        
        const uint8_t add = (isHighRes*2);
        if( i+4+add > length )
            break;

        char valString[3+add] = {0};
        strlcpy(valString, payload+i+2, 3+add);
        uint16_t val = strtoul(valString, 0, 16); // Assigns 32bit to 16 bit, but that's because there's no unsigned strto function for 16bit
        setFlatPWM(targetMotor, val, isHighRes);

        i += 4+add;

    }


}


void ApiClient::setFlatPWM( uint8_t motor, uint16_t value, bool highRes ){
    motors[motor].stopProgram();  // Stop any running program when this is received
    motors[motor].setPWM(value, highRes);
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
