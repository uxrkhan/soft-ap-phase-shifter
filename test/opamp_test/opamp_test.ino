#include <Wire.h>

#define ADDR 0x62
#define VCC 3.27
#define GAIN 3.2

void setup() {
  pinMode(2,OUTPUT);
  Serial.begin(115200);
  Wire.begin();
  Wire1.begin(14,13);
  delay(1000);
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
  Serial.print("Enter voltage: ");
  while (Serial.available() == 0) {
    
  }
  String voltage_str = Serial.readString();
  Serial.println(voltage_str);
  Serial.println("Voltage entered: " + voltage_str + " V");
  
  float voltage = voltage_str.toFloat();
  Serial.print("Float voltage: ");
  Serial.println(voltage);

  int val = int(voltage * 4096 / 3.3);
  if (val > 4095) 
    val = 4095;
  if (val < 0)
    val = 0;

  float dac_op = val * VCC / 4096;
  float output = dac_op * GAIN;
  Serial.print("INDEX VALUE:     ");
  Serial.println(val);
  Serial.print("DAC OUTPUT:      ");
  Serial.println(dac_op);
  Serial.print("OPAMP OUTPUT:    ");
  Serial.println(output);
  Serial.println("--------------");

  send(ADDR, val);
  Serial.println("Voltage successfully transferrred.");
}
