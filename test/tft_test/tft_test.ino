#include <TFT_eSPI.h>
#include <SPI.h>

#define TFT_BL 15

TFT_eSPI tft = TFT_eSPI();

void setup() {
//  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, 255);
  tft.init();
  tft.setRotation(1);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN);

  tft.fillScreen(TFT_BLACK);
  tft.drawString("Hello world!", 3, 3);
}

void loop() {
  
}
