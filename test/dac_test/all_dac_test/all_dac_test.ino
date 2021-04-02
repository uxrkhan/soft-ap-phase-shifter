#include <Wire.h>

#define dac1A 0x60  // different manufacturer
#define dac1B 0x63  // soldered
#define dac2A 0x62  // default
#define dac2B 0x63  // soldered

float voltages[] = {1, 2, 2.5, 3};
int vals[4];

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire1.begin(14,13);

  for (int i = 0; i < 4; i++) {
    vals[i] = floor(voltages[i] * (4095/11.65));
    float Vdac = voltages[i] / 11.65 * 3.3;
    Serial.println(String(vals[i]) + " " + String(Vdac));
  }
  send1(dac1A, vals[0]);
  send1(dac1B, vals[1]);
  send2(dac2A, vals[2]);
  send2(dac2B, vals[3]);
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

void loop()
{  
   
}
