#include <Wire.h>

#define Vmax 12
#define Vcc 3.3

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire1.begin(14,13);
  
  float Vin = 5.00;
  int val = int(4095 * Vin / Vmax);
  send(0x60, val);
  send(0x63, val);
  send2(0x62, val);
  send2(0x63, val);
  delay(4000);
}

void send(byte addr, int val) {
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

void loop() {
//  float Vin = 1.00;
//  int val = int(4095 * Vin / Vmax);
//  send(0x60, val);
//  send(0x63, val);
//  send2(0x62, val);
//  send2(0x63, val);
//  delay(4000);

//  Vin = 5.00;
//  val = int(4095 * Vin / Vmax);
//  send(0x60, val);
//  send(0x63, val);
//  send2(0x62, val);
//  send2(0x63, val);
//  delay(4000);
  
//  Serial.println("Enter voltage...");
//  while(Serial.available() == 0) {
//  }
//  float Vin = Serial.readString().toFloat();
//  int val = int(4095 * Vin / Vmax);
//  float Vd = val * 3.3 / 4095;
//  send(0x60, val);
//  send(0x63, val);
//  send2(0x62, val);
//  send2(0x63, val);
//  Serial.println("Voltage entered: " + String(Vin) + " V");
//  Serial.println("Index value: " + String(val));
//  Serial.println("DAC output: " + String(Vd) + " V");
//  Serial.println("Sent voltage.");
}
