#include <TFT.h>
#include <SPI.h>

#define CS D2
#define DC 10
#define RST 11

TFT TFTscreen = TFT(CS, DC, RST);

char txt[4];

void setup() {
	TFTscreen.begin();
	TFTscreen.background(0,0,0);	// black background

	TFTscreen.stroke(255, 255, 255);	// white stroke
	TFTscreen.setTextSize(2);
	TFTscreen.text("Value: ", 0, 0);

	TFTscreen.setTextSize(5);		// bigger font size for value
}

void loop() {
	String val = String(analogRead(A0));
	val.toCharArray(txt, 4);

	// write the text in white color
	TFTscreen.stroke(255, 255, 255);
	TFTscreen.text(txt, 0, 20);

	delay(200);

	// erase the text by overwriting in black
	TFTscreen.stroke(0, 0, 0);
	TFTscreen.text(txt, 0, 20);
}
