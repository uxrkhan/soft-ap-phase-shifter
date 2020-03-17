#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <Wire.h>

#define dac1 0x62
#define dac2 0x63

const char* ssid = "esp";
const char* password =  "esppse32";
int queries;
int dataLog[5];
int vals[4];

AsyncWebServer server(80);

void processFormParams(AsyncWebServerRequest *request) {
  int paramsNr = request->params();
  queries = paramsNr;
  Serial.println(paramsNr);

  for(int i=0; i<paramsNr; i++){
    AsyncWebParameter* p = request->getParam(i);
    Serial.print("Parameter Name: ");
    Serial.println(p->name());
    Serial.print("Parameter Value: ");
    Serial.println(p->value());
    Serial.println("------");
    dataLog[i] = (p->value()).toInt();
  }
}

void setup(){
  Serial.begin(115200);
 
  if(!SPIFFS.begin()){
     Serial.println("An Error has occurred while mounting SPIFFS");
     return;
  }
  
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("IP Address: ");
  Serial.println(IP);
  
  server.begin();
 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send(SPIFFS, "/index.html", "text/html"); 
  });
  server.on("/csstest.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/csstest.css", "text/css");
  });
  server.on("/bg.jpg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bg.jpg", "text/html");
  });
  server.on("/case1.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/case1.html", "text/html");
    processFormParams(request);  
  });
  server.on("/case2.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/case2.html", "text/html");
    processFormParams(request);
  });
  
  server.begin();
  Wire.begin();
  Wire1.begin(19,18,100000);
}

void send1(byte addr, int val) {
  Wire.beginTransmission(addr);
  Wire.write(64);
  Wire.write(val >> 4);
  Wire.write(val << 4);
  Wire.endTransmission();
}

void send2(byte addr, int val) {
  Wire1.beginTransmission(addr);
  Wire1.write(64);
  Wire1.write(val >> 4);
  Wire1.write(val << 4);
  Wire1.endTransmission();
}

void loop(){
  if(queries == 4){
    for (int i = 0; i < 4; i++) {
      vals[i] = ceil(dataLog[i] * (4095/3.3));
    }
    send1(dac1, vals[0]);
    send1(dac2, vals[1]);
    send2(dac1, vals[2]);
    send2(dac2, vals[3]); 
  }
  else if (queries == 5) {
    
  }
}
