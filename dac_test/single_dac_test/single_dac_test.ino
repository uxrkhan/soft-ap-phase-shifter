// This is a program to test a single MCP4725 DAC 
// Enter the input voltage in the terminal and the program will display the corresponding voltages. 

#include <Wire.h>

#define ADDR 0x63   // dac address (0x63 -> dac with short circuit)
#define VCC 3.63    // measure voltage across dac VCC and enter here.
#define GAIN 3.7    // op-amp gain

void setup() {
  Wire.begin();
  Serial.begin(115200);
}

void send(byte addr, int val) {
  Wire.beginTransmission(addr);
  Wire.write(64);
  Wire.write(val >> 4);
  Wire.write(val << 4);
  Wire.endTransmission();
}

void loop() {
  Serial.print("Enter voltage. ");
  while(Serial.available()==0) { 
  }
  String voltage_str = Serial.readString();
  Serial.println(voltage_str);

  Serial.println("--------------");
  Serial.print("VOLTAGE ENTERED: ");
  Serial.print(voltage_str);

  float voltage = voltage_str.toFloat();
  
  Serial.print("FLOAT VOLTAGE:   ");
  Serial.println(voltage);

  int val = int(voltage * 4096 / 12) % 4096;  // mod because after 4095 is 0
  float dac_op = val * VCC / 4096;
  float output = dac_op * GAIN;

  Serial.print("INDEX VALUE:     ");
  Serial.println(val);
  Serial.print("DAC OUTPUT:      ");
  Serial.println(dac_op);
  Serial.print("OPAMP OUTPUT:    ");
  Serial.println(output);
  Serial.println("--------------\n");

  send(ADDR, val);
  Serial.println("Voltage successfully transferred.\n");
}
