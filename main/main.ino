#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <Wire.h>
//#include <TFT_eSPI.h>
#include <SPI.h>
#include <vector>
using namespace std;

#define Vmax 11.65

#define SSID "ESP32"
#define PASS "espesp32"

#define dac1a 0x60
#define dac1b 0x63
#define dac2a 0x62
#define dac2b 0x63

#define SDA2 14
#define SCL2 13
// #define TFT_BL 15

#define A 27
#define B 26
#define C 2
#define D 4
#define E 5
#define F 25
#define G 33
#define DP 15

/* connections */
// SCL1 22
// SDA1 21
// SCL2 13
// SDA2 14
/* if seven segment is used */
// A    27
// B    26
// C     2
// D     4
// E     5
// F    25
// G    33
// DP   15
/* if TFT display is used */
// DIN  23
// CS    2
// SCK  18
// DC    5
// BL   15
// RST   4

float Vctl[][4] = {{ 0.00,  0.00,  0.00,  0.00},
  { 2.82,  1.62,  0.83,  0.00},
  { 7.21,  4.35,  1.65,  0.00},
  {11.65,  6.82,  0.75,  0.00},
  { 0.00,  0.83,  1.62,  2.82},
  { 0.00,  1.65,  4.35,  7.21},
  { 0.00,  0.75,  6.82, 11.65},
  { 1.65,  0.00,  0.00,  1.65}
};

int ang[] = {0, 15, 30, 45, -15, -30, -45, 30};

short ss[] = {A, B, C, D, E, F, G, DP};

short  _case = -1;
String _casename;
int    _interval = 1;
int    _scanangle = 0;
bool   _inprocess = false;
int    _nofangles;
vector<int> _row_order;
vector<int> _angle_order;

AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();

// helper functions

int getParamInt(AsyncWebServerRequest *request, String name) {
  AsyncWebParameter* p = request->getParam(name);
  int value = (p->value()).toInt();
  return value;
}

float getParamFloat(AsyncWebServerRequest *request, String name) {
  AsyncWebParameter* p = request->getParam(name);
  float value = (p->value()).toFloat();
  return value;
}

int getRow(int scanAngle) {
  int row = 0;
  if (scanAngle == 15)       row = 1;
  else if (scanAngle == 30)  row = 2;
  else if (scanAngle == 45)  row = 3;
  else if (scanAngle == -15) row = 4;
  else if (scanAngle == -30) row = 5;
  else if (scanAngle == -45) row = 6;
  return row;
}

//void drawValues(float Vport[], String title, int scanAngle) {
//  int xs = 3, ys = 25;
//  tft.fillRect(xs, ys, 160, 84, TFT_BLACK);
//  tft.setTextColor(TFT_WHITE);
//  tft.drawString(title, xs, ys);
//  tft.setTextColor(TFT_GREEN);
//  tft.setTextSize(1);
//  int angles[4] = {0};
//  if (title == "Dual Beam Scan" || title == "Continuous Dual Beam Scan") {
//    angles[0] = scanAngle;
//    angles[3] = -scanAngle;
//  } else if (title == "Manual") {
//    for (int i = 0; i < 4; i++)
//      angles[i] = -1;
//  } else {
//    for (int i = 0; i < 4; i++) {
//      angles[i] = i * scanAngle;
//    }
//  }
//  tft.drawString(String("PORT 1 > ") + String(Vport[0]) + " V   " + String(angles[0]) + " deg", xs, ys + 13);
//  tft.drawString(String("PORT 2 > ") + String(Vport[1]) + " V   " + String(angles[1]) + " deg", xs, ys + 33);
//  tft.drawString(String("PORT 3 > ") + String(Vport[2]) + " V   " + String(angles[2]) + " deg", xs, ys + 53);
//  tft.drawString(String("PORT 4 > ") + String(Vport[3]) + " V   " + String(angles[3]) + " deg", xs, ys + 73);
//}

void sendVoltages(float Vport[], String title, int scanAngle) {
  int vals[4] = {0, 0, 0, 0};
  float dac_op[4] = {0, 0, 0, 0};
  for (int i = 0; i < 4; i++) {
    vals[i] = floor((Vport[i] / Vmax) * 4095);
    dac_op[i] = Vport[i] / Vmax * 3.3;
    //	  Serial.println("PORT " + String(i+1) + ": " + String(Vport[i]) + " V  i=" + String(vals[i]) + "  Vdac=" + String(dac_op[i]) );
  }
  send1(dac1a, vals[0]);
  send1(dac1b, vals[1]);
  send2(dac2a, vals[2]);
  send2(dac2b, vals[3]);
//  drawValues(Vport, title, scanAngle);
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

vector<int> str2order(String str) {
  vector<int> order;
  char str2[str.length() + 1];
  str.toCharArray(str2, str.length() + 1);
  char* temp = strtok(str2, " ");
  while (temp) {
    int row = getRow(String(temp).toInt());
    order.push_back(row);
    temp = strtok(NULL, " ");
  }
  return order;
}

void segment_disp(short num) {
  if (num < 0 || num > 9) {
    return;
  }
  bool truth[][8] = {{0,0,0,0,0,0,1,1},
                     {1,0,0,1,1,1,1,1},
                     {0,0,1,0,0,1,0,1},
                     {0,0,0,0,1,1,0,1},
                     {1,0,0,1,1,0,0,1},
                     {0,1,0,0,1,0,0,1},
                     {0,1,0,0,0,0,0,1},
                     {0,0,0,1,1,1,1,1},
                     {0,0,0,0,0,0,0,1},
                     {0,0,0,0,1,0,0,1}};
                     
  for (int i = 0; i < 8; i++) {
    digitalWrite(ss[i], truth[num][i]);
  }
}

// main functions

void setup() {
  // initialize serial connections
  
  Serial.begin(115200);
  Wire.begin();
  Wire1.begin(SDA2, SCL2);

  send1(dac1a, 0);
  send1(dac1b, 0);
  send2(dac2a, 0);
  send2(dac2b, 0);

  // setup seven segment pins
  for (int i = 0; i < 8; i++) {
    pinMode(ss[i], OUTPUT);
    digitalWrite(ss[i], LOW);
  }

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // initialise wifi
  WiFi.softAP(SSID, PASS);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("IP Address: ");
  Serial.println(IP);

  String ipString = "";
  for (int i = 0; i < 4; i++) {
    ipString += i ? "." + String(IP[i]) : String(IP[i]);
  }
  
//  pinMode(TFT_BL, OUTPUT);
//  digitalWrite(TFT_BL, HIGH);
//  tft.init();
//  tft.setRotation(1);
//  tft.fillScreen(TFT_BLACK);
//  tft.setTextSize(1);
//  tft.setTextColor(TFT_GREEN);
//  tft.drawString("SSID: " + String(SSID), 3, 3);
//  tft.drawString(ipString, 3, 13);
//  tft.drawRect(1, 23, 160, 1, TFT_GREEN);

  server.begin();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    _inprocess = false;
    request->send(SPIFFS, "/index.html", "text/html");
    if (request->hasParam("clear")) {
      // case 0
      _case = 0;
      _scanangle = 0;
      _casename = "Clear";
      sendVoltages(Vctl[0], "Clear", 0);
      segment_disp(_case);
      Serial.println("GET: CLEAR");
    } else if (request->hasParam("scan-angle-case1")) {
      // case 1
      _case = 1;
      _casename = "Main Beam Scan";
      _scanangle = getParamInt(request, "scan-angle-case1");
      int row = getRow(_scanangle);
      segment_disp(_case);
      Serial.println("GET: CASE 1 " + String(_scanangle) + " deg");
      sendVoltages(Vctl[row], "Main Beam Scan", _scanangle);
    } else if (request->hasParam("scan-angle-case2")) {
      // case 2
      _case = 2;
      _casename = "Dual Beam Scan";
      _scanangle = getParamInt(request, "scan-angle-case2");
      Serial.println("GET: CASE 2 " + String(_scanangle) + " deg");
      sendVoltages(Vctl[7], "Dual Beam Scan", _scanangle);
      segment_disp(_case);
    } else if (request->hasParam("scan-angle-case3")) {
      // case 3
      _scanangle = getParamInt(request, "scan-angle-case3");
      _interval = getParamInt(request, "interval") * 1000;
      Serial.println("GET: CASE 3 " + String(_scanangle) + " deg " + String(_interval) + " s");
      _casename = "Continuous Beam Scan";
      _angle_order = {1, 2, 3, 0};
      if (_scanangle == 15)
        _row_order = {1, 2, 3, 0};
      else
        _row_order = {4, 5, 6, 0};
      _case = 3;
      _inprocess = true;
      segment_disp(_case);
    } else if (request->hasParam("scan-angle-case4")) {
      // case 4
      _scanangle = getParamInt(request, "scan-angle-case4");
      _interval = getParamInt(request, "interval") * 1000;
      Serial.println("GET: CASE 4 " + String(_scanangle) + " deg " + String(_interval) + " s");
      _casename = "Continuous Dual Beam Scan";
      _case = 4;
      _inprocess = true;
      segment_disp(_case);
    } else if (request->hasParam("scan-angle-case5")) {
      // case 5
      _interval = getParamInt(request, "interval") * 1000;
      _casename = "Random Scan Order";
      String inputValuesString = (request->getParam("scan-order"))->value();
      Serial.println("GET: CASE 5 {" + inputValuesString + "} deg " + String(_interval) + " s");
      _row_order = str2order(inputValuesString);
      _nofangles = _row_order.size();
      _angle_order.clear();
      for (int i = 0; i < _nofangles; i++) {
        _angle_order.push_back(ang[_row_order[i]]);
      }
      _case = 5;
      _inprocess = true;
      segment_disp(_case);
    } else if (request->hasParam("manual")) {
      // manual
      _case = 6;
      Serial.println("GET: CASE 6: MANUAL");
      float Vman[4] = {0};
      Vman[0] = getParamFloat(request, "port1");
      Vman[1] = getParamFloat(request, "port2");
      Vman[2] = getParamFloat(request, "port3");
      Vman[3] = getParamFloat(request, "port4");
      sendVoltages(Vman, "Manual", -1);
      segment_disp(_case);
    }
  });

  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/styles.css", "text/css");
  });

  server.begin();

  for (int i = 9; i >= 0; i--) {
    segment_disp(i);
    delay(100);
  }

}

void loop() {
  // cases that require switching
  if (_case == 3) {
    while (_inprocess && _case == 3) {
      for (int i = 0; i < 4; i++) {
        if (!_inprocess || _case != 3) {
          break;
        }
        sendVoltages(Vctl[_row_order[i]], _casename, _scanangle * _angle_order[i]);
        delay(_interval);
      }
      if (!_inprocess || _case != 3) break;
    }
  } else if (_case == 4) {
    while (_inprocess && _case == 4) {
      if (!_inprocess || _case != 4) {
        break;
      }
      sendVoltages(Vctl[7], _casename, _scanangle);
      delay(_interval);
      if (!_inprocess || _case != 4)  {
        break;
      }
      sendVoltages(Vctl[0], _casename, 0);
      delay(_interval);
    }
  } else if (_case == 5) {
    while (_inprocess && _case == 5) {
      for (int i = 0; i < _nofangles; i++) {
        if (!_inprocess || _case != 5) break;
        sendVoltages(Vctl[_row_order[i]], _casename, _angle_order[i]);
        delay(_interval);
      }
      if (!_inprocess || _case != 5) break;
    }
  }
}
