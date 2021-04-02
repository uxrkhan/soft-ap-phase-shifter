#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#define Vmax  11.65

#define dac1a 0x60
#define dac1b 0x63
#define dac2a 0x62
#define dac2b 0x63

#define SDA2 14
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
bool updateFlag = false;
float Vctl[][4] = {{ 0.00,  0.00,  0.00,  0.00},
                   { 2.82,  1.62,  0.83,  0.00},
                   { 7.21,  4.35,  1.65,  0.00},
                   {11.65,  6.82,  0.75,  0.00},
                   { 0.00,  0.83,  1.62,  2.82},
                   { 0.00,  1.65,  4.35,  7.21},
                   { 0.00,  0.75,  6.82, 11.65},
                   { 1.65,  0.00,  0.00,  1.65}};
int row = 0;
 
AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();

int getScanAngle(AsyncWebServerRequest *request) {
  AsyncWebParameter* p = request->getParam(0);
  String scanAngleStr = p->value();
  int scanAngle = scanAngleStr.toInt();
  Serial.println(String(p->name()) + ": " + String(scanAngle));
  return scanAngle;
}

void drawValues(String title, int angle) {
  int xs = 3, ys = 25;
  tft.setTextColor(TFT_WHITE);
  tft.drawString(title, xs, ys);
  tft.setTextColor(TFT_GREEN);
  tft.fillRect(xs, ys, 160, 84, TFT_BLUE);
  tft.setTextSize(1);
  int angles[4] = {0};
  if (title == "Main Beam Scan") {
    for (int i = 0; i < 4; i++) 
      angles[i] = i * angle;
  } else {
    angles[0] = angle;
    angles[3] = angle;
  }
  tft.drawString(String("PORT 1 > ") + String(Vctl[row][0]) + " V   " + String(angles[0]) + "°", xs, ys + 13);
  tft.drawString(String("PORT 2 > ") + String(Vctl[row][1]) + " V   " + String(angles[1]) + "°", xs, ys + 33);
  tft.drawString(String("PORT 3 > ") + String(Vctl[row][2]) + " V   " + String(angles[2]) + "°", xs, ys + 53);
  tft.drawString(String("PORT 4 > ") + String(Vctl[row][3]) + " V   " + String(angles[3]) + "°", xs, ys + 73);
}

void setup(){
  Serial.begin(115200);
  Wire.begin();
  Wire1.begin(SDA2,SCL2);
 
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
    // processFormParams(request);  
    int n_params = request->params();
    if (n_params > 0) { 
      int scanAngle = getScanAngle(request);
      if (scanAngle == 15)       row = 1;
      else if (scanAngle == 30)  row = 2;
      else if (scanAngle == 45)  row = 3;
      else if (scanAngle == -15) row = 4; 
      else if (scanAngle == -30) row = 5;
      else if (scanAngle == -45) row = 6;
      else                       row = 0;
      updateFlag = true;
      drawValues("Main Beam Scan", scanAngle);
    }
  });
  server.on("/case2.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/case2.html", "text/html");
    if (request->params()) {
      int scanAngle = getScanAngle(request);
      if (scanAngle == 30 || scanAngle == -30) 
        row = 7;
      else 
        row = 0;
      updateFlag = true;
      drawValues("Dual Beam Scan", scanAngle);
    }
  });

  server.begin();
  String ipString = "";
  for (int i = 0; i < 4; i++) {
    ipString += i ? "." + String(IP[i]) : String(IP[i]);  
  }
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("SSID: " + String(ssid), 3, 3);
  tft.drawString(ipString, 3, 13);
  tft.drawRect(1, 23, 160, 1, TFT_GREEN);
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
  if (updateFlag) {
    int vals[4] = {0, 0, 0, 0};
    float dac_op[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++) {
      vals[i] = floor((Vctl[row][i] / Vmax) * 4095);
      dac_op[i] = Vctl[row][i] / Vmax * 3.3;
      Serial.println("PORT " + String(i+1) + ": " + String(Vctl[row][i]) + " " + String(vals[i]) + " " + String(dac_op[i]));
    }
    send1(dac1a, vals[0]);
    send1(dac1b, vals[1]);
    send2(dac2a, vals[2]);
    send2(dac2b, vals[3]);     
    updateFlag = false; 
  }
}
