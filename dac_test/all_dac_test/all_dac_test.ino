#include <Wire.h>

#define dac1 0x62
#define dac2 0x63  

float voltages[] = {1, 2, 2.5, 3.3};
int vals[4];

void setup() {
  Wire.begin();
  Wire1.begin(18,19,100000);
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
  for (int i = 0; i < 4; i++) {
    vals[i] = ceil(voltages[i] * (4095/3.3));
  }
  send1(dac1, vals[0]);
  send1(dac2, vals[1]);
  send2(dac1, vals[2]);
  send2(dac2, vals[3]); 
}
