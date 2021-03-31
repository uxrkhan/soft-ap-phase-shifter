#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#define dac1a 0x60
#define dac1b 0x63
#define dac2a 0x62
#define dac2b 0x63

#define SDA2 12
#define SCL2 13

// connections
// SCL1 22
// SDA1 21
// SCL2 13
// SDA2 14
// DIN  23
// CS    2
// SCK  18
// DC    5
// BL   15
// RST   4 

const char* ssid = "ESP32";
const char* password =  "espesp32";
int queries;
float dataLog[5];
int vals[4];

AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();

void processFormParams(AsyncWebServerRequest *request) {
  int paramsNr = request->params();
  queries = paramsNr;
  Serial.println("# params: " + String(paramsNr));

  for(int i=0; i<paramsNr; i++){
    AsyncWebParameter* p = request->getParam(i);
    Serial.print("Parameter Name: ");
    Serial.println(p->name());
    Serial.print("Parameter Value: ");
    Serial.println(p->value());
    dataLog[i] = (p->value()).toFloat();
  }
  Serial.println("------");

  drawValues();
}

void drawValues() {
  int xs = 3, ys = 36;
  tft.fillRect(xs, ys, 160, 84, TFT_BLUE);
  tft.setTextSize(2);
  tft.drawString(String("PORT 1 > ") + String(dataLog[0]), xs, ys + 3);
  tft.drawString(String("PORT 2 > ") + String(dataLog[1]), xs, ys + 23);
  tft.drawString(String("PORT 3 > ") + String(dataLog[2]), xs, ys + 43);
  tft.drawString(String("PORT 4 > ") + String(dataLog[3]), xs, ys + 63);
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
     AsyncWebParameter* p = request->getParam(0);
    String scanAngleStr = p->value();
    int scanAngle = scanAngleStr.toInt();
    Serial.println(String(p->name()) + ": " + String(scanAngle));
    // processFormParams(request);  
  });
  server.on("/case2.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/case2.html", "text/html");
    processFormParams(request);
  });
  
  server.begin();
  Wire.begin();
  Wire1.begin(SDA2,SCL2,100000);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("SSID: " + String(ssid), 3, 3);
  tft.drawString("PASS: " + String(password), 3, 13);
  tft.drawString("IP  : " + String(IP), 3, 23);
  tft.drawRect(1, 33, 160, 1, TFT_GREEN);
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
//  if(queries == 4){
//    // static input
//    for (int i = 0; i < 4; i++) {
//      vals[i] = floor(dataLog[i] * (4095/3.3));
//    }
//    send1(dac1a, vals[0]);
//    send1(dac1b, vals[1]);
//    send2(dac2a, vals[2]);
//    send2(dac2b, vals[3]); 
//  }
//  else if (queries == 5) {
//    
//  }
}
