# soft-ap-phase-shifter

Software enabled access point to control voltage at a load wirelessly.

Project in construction :)

## How to run it

1. Make the connections.
2. Connect the ESP32 to the computer and make sure it is visible. If it's not visible, try with a different cable.
3. Click on `Tools > ESP32 Sketch Data Upload` to upload the html and css files. 
4. Verify and the upload the code to ESP32 (**NOTE: While uploading, make sure to press and hold BOOT button on ESP32**).
5. Open the Arduino Serial Monitor and press "EN" on the ESP32 to run the program.
6. Connect to the access point from your device using the SSID and password given in the main code.
5. Open browser and enter the IP address shown in the Serial Monitor.

## How it works

Since there are 4 different ports, we use 4 DACs, 2 with address 0x62 and 2 with address 0x63.
```
PORT ADDR   I2C
1    0x62   Wire
2    0x63   Wire
3    0x62   Wire1 (software)
4    0x63   Wire1 (software)
```
Wire1 is a software I2C and is connected to pins 19 and 18.

DAC will have address 0x62 by default.
DAC will have address 0x63 if the ADD pin is short circuited to VCC pin on the MCP4725.

Input from the client would be in the range [0,13]. This gives a step size of 13/4096 = 0.0031948. This is then converted the output from the DAC will be in the range [0,3.3] V. This will be amplified to [0,13] V range. V range.
