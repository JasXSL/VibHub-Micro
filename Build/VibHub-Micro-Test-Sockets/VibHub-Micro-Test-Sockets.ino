#include <WiFi.h>
#include <SocketIoClient.h>


/////////////////////////////////////
////// USER DEFINED VARIABLES //////
///////////////////////////////////
/// WIFI Settings ///
const char* ssid     = "DogeSuchWOW";
const char* password = "doge is love doge is life";

/// Socket.IO Settings ///
char host[] = "vibhub.io"; // Socket.IO Server Address
int port = 80; // Socket.IO Port Address
char path[] = "/socket.io/?transport=websocket"; // Socket.IO Base Path

/////////////////////////////////////
////// ESP32 Socket.IO Client //////
///////////////////////////////////

SocketIoClient webSocket;
WiFiClient client;

void socket_Connected(const char * payload, size_t length) {
  Serial.println("Socket.IO Connected!");
}

void socket_statusCheck(const char * payload, size_t length) {
  const char* message = "\"ON\"";
  webSocket.emit("status", message);
}

void socket_event(const char * payload, size_t length) {
  Serial.print("got message: ");
  Serial.println(payload);
}



void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup 'on' listen events
  webSocket.on("connect", socket_Connected);
  webSocket.on("event", socket_event);
  webSocket.on("status", socket_statusCheck);


	webSocket.begin(host, port, path);


}

void loop() {
  webSocket.loop();
}