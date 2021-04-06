#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#define Vmax 11.65

#define SSID "ESP32"
#define PASS "espesp32"

#define dac1a 0x60
#define dac1b 0x63
#define dac2a 0x62
#define dac2b 0x63

#define SDA2 14
#define SCL2 13
#define TFT_BL 15

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

float Vctl[][4] = {{ 0.00,  0.00,  0.00,  0.00},
				   { 2.82,  1.62,  0.83,  0.00},
				   { 7.21,  4.35,  1.65,  0.00},
				   {11.65,  6.82,  0.75,  0.00},
				   { 0.00,  0.83,  1.62,  2.82},
				   { 0.00,  1.65,  4.35,  7.21},
				   { 0.00,  0.75,  6.82, 11.65},
				   { 1.65,  0.00,  0.00,  1.65}};

short  _case = -1;
String _casename;
int    _interval = 1;
int    _scanangle = 0;
bool   _inprocess = false;

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

void drawValues(float Vport[], String title, int scanAngle) {
	int xs = 3, ys = 25;
	tft.setTextColor(TFT_WHITE);
	tft.drawString(title, xs, ys);
	tft.setTextColor(TFT_GREEN);
	tft.fillRect(xs, ys, 160, 84, TFT_BLUE);
	tft.setTextSize(1);
	int angles[4] = {0};
	if (title == "Main Beam Scan" || title == "Continuous Beam Scan") {
		for (int i = 0; i < 4; i++) 
		angles[i] = i * scanAngle;
	} else if (title == "Dual Beam Scan" || title == "Continuous Dual Beam Scan") {
		angles[0] = scanAngle;
		angles[3] = -scanAngle;
	} else if (title == "Manual") {
		for (int i = 0; i < 4; i++) 
		  angles[i] = -1;
	}
	tft.drawString(String("PORT 1 > ") + String(Vport[0]) + " V   " + String(angles[0]) + "*", xs, ys + 13);
	tft.drawString(String("PORT 2 > ") + String(Vport[1]) + " V   " + String(angles[1]) + "*", xs, ys + 33);
	tft.drawString(String("PORT 3 > ") + String(Vport[2]) + " V   " + String(angles[2]) + "*", xs, ys + 53);
	tft.drawString(String("PORT 4 > ") + String(Vport[3]) + " V   " + String(angles[3]) + "*", xs, ys + 73);
}

void sendVoltages(float Vport[], String title, int scanAngle) {
	int vals[4] = {0, 0, 0, 0};
		float dac_op[4] = {0, 0, 0, 0};
  Serial.println("Sending voltages for case: " + title);
	for (int i = 0; i < 4; i++) {
	  vals[i] = floor((Vport[i] / Vmax) * 4095);
	  dac_op[i] = Vport[i] / Vmax * 3.3;
	  Serial.println("PORT " + String(i+1) + ": " + String(Vport[i]) + " V  i=" + String(vals[i]) + "  Vdac=" + String(dac_op[i]) );
	}
	send1(dac1a, vals[0]);
	send1(dac1b, vals[1]);
	send2(dac2a, vals[2]);
	send2(dac2b, vals[3]);
	drawValues(Vport, title, scanAngle);
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

// main functions

void setup() {
	// initialize serial connections
	Serial.begin(115200);
	Wire.begin();
	Wire1.begin(SDA2,SCL2);

	if(!SPIFFS.begin()){
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

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("SSID: " + String(SSID), 3, 3);
  tft.drawString(ipString, 3, 13);
  tft.drawRect(1, 23, 160, 1, TFT_GREEN);
  
	server.begin();

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    _inprocess = false;
		request->send(SPIFFS, "/index.html", "text/html");
		if (request->hasParam("clear")) {
			// case 0
      _case = 0;
			sendVoltages(Vctl[0], "Clear", 0);
		} else if (request->hasParam("scan-angle-case1")) {
			// case 1
      _case = 1;
			int scanAngle = getParamInt(request, "scan-angle-case1");
			int row = getRow(scanAngle);
			sendVoltages(Vctl[row], "Main Beam Scan", scanAngle);
		} else if (request->hasParam("scan-angle-case2")) {
			// case 2
      _case = 2;
			int scanAngle = getParamInt(request, "scan-angle-case2");
			sendVoltages(Vctl[7], "Dual Beam Scan", scanAngle);
		} else if (request->hasParam("scan-angle-case3")) {
			// case 3
			_scanangle = getParamInt(request, "scan-angle-case3");
			_interval = getParamInt(request, "interval") * 1000;
			_casename = "Continuous Beam Scan";
			_case = 3;
      _inprocess = true;
		} else if (request->hasParam("scan-angle-case4")) {
			// case 4
			_scanangle = getParamInt(request, "scan-angle-case4");
			_interval = getParamInt(request, "interval") * 1000;
			_casename = "Continuous Dual Beam Scan";
      _case = 4;
      _inprocess = true;
		} else if (request->hasParam("scan-angle-case5")) {
			// case 5
      _case = 5;
			String inputValuesString = (request->getParam("scan-order"))->value();
			int interval_ms = getParamInt(request, "interval") * 1000;
			String title = "Random Scan Order";
			// int order[] = str2order(inputValuesString);
			// for (int i = 0; )

		} else if (request->hasParam("manual")) {
			// manual
			float Vman[4] = {0};
			Vman[0] = getParamFloat(request, "port1");
			Vman[1] = getParamFloat(request, "port2");
			Vman[2] = getParamFloat(request, "port3");
			Vman[3] = getParamFloat(request, "port4");
			sendVoltages(Vman, "Manual", -1);
		}
	});

  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/styles.css", "text/css");
  });

  server.begin();

}

void loop() {
  if (_case == 3) {
    if (_scanangle == 15) {
      while(_inprocess) {
        if (!_inprocess) break;
        sendVoltages(Vctl[1], _casename, _scanangle * 1);
        delay(_interval);
        if (!_inprocess) break;
        sendVoltages(Vctl[2], _casename, _scanangle * 2);
        delay(_interval);
        if (!_inprocess) break;
        sendVoltages(Vctl[3], _casename, _scanangle * 3);
        delay(_interval);
        if (!_inprocess) break;
        sendVoltages(Vctl[0], _casename, _scanangle * 0);
        delay(_interval);
      }
    } else if (_scanangle == -15) {
      while(_inprocess) {          
        if (!_inprocess) break;
        sendVoltages(Vctl[4], _casename, _scanangle * 1);
        delay(_interval);
        if (!_inprocess) break;
        sendVoltages(Vctl[5], _casename, _scanangle * 2);
        delay(_interval);
        if (!_inprocess) break;
        sendVoltages(Vctl[6], _casename, _scanangle * 3);
        delay(_interval);
        if (!_inprocess) break;
        sendVoltages(Vctl[0], _casename, _scanangle * 0);
        delay(_interval);
      }
    }
  } else if (_case == 4) {
    while(_inprocess) {          
      if (!_inprocess) break;
      sendVoltages(Vctl[7], _casename, _scanangle);
      delay(_interval);
      if (!_inprocess) break;
      sendVoltages(Vctl[0], _casename, _scanangle);
      delay(_interval);
    }
  }
}
