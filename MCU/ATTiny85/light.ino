#include "Arduino.h"
#include "TinyWireS.h"                  // wrapper class for I2C slave routines
#include <Adafruit_NeoPixel.h>



#define I2C_SLAVE_ADDR 0x2E             // I2C slave address (46, can be changed)

#define PB1 1
#define ADC2 2

#define STDPB1_1 0xE9 // Command to Set ATtiny85 PB1 = 1
#define STDPB1_0 0xE1 // Command to Set ATtiny85 PB1 = 0
#define STANAPB3 0xFB // Command to Set ATtiny85 PB3 = PWMx
#define READADC2 0xDA // Command to Read ATtiny85 ADC2

//// aquarium variables init ///

#define CLRUPDT 0x52 // Command to Read Change Color
byte color[4] = {0};

#define PIN       3
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//// aquarium variables end ///

byte command[6] = { 0 }; 
bool newCommand = false;



void setup() {
	pixels.begin();
	pixels.clear();

	TinyWireS.begin(I2C_SLAVE_ADDR); 
	TinyWireS.onReceive(receiveEvent);

	pinMode(PB1, OUTPUT);
}


void loop() {
	if(newCommand)
		options();
	pixels.show();

}

void receiveEvent(byte commandbytes) {
	for (int i = 0; i < int(commandbytes); i++) {
		command[i] = TinyWireS.receive();
	}
	newCommand = true;
}


void options() {
	switch (command[0]) {
		case STDPB1_1: {
			pixels.setPixelColor(0, pixels.Color(255, 150, 255));
			digitalWrite(PB1, HIGH);   
			break;
		}
		case STDPB1_0: {
			pixels.clear();
			digitalWrite(PB1, LOW);    
			break;
		}
		case CLRUPDT: {
			color[0] = command[1];
			color[1] = command[2];
			color[2] = command[3];
			color[3] = command[4];
			digitalWrite(PB1, HIGH);   
			for(int i=0; i<NUMPIXELS; i++) { 
				pixels.setPixelColor(i, pixels.Color(color[0], color[1], color[2]));
			}
			pixels.setBrightness(color[3]);
			break;
		}
		default: {
			break;
		}
	}
	newCommand = false;
}


