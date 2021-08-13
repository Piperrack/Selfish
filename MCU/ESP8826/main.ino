#include "Arduino.h"
#include <Wire.h>

#define SLAVE_RESET_PIN 2

#define STDPB1_1 0xE9 // Command to Set ATtiny85 PB1 = 1
#define STDPB1_0 0xE1 // Command to Set ATtiny85 PB1 = 0
#define STANAPB3 0xFB // Command to Set ATtiny85 PB3 = PWMx
#define READADC2 0xDA // Command to Read ATtiny85 ADC2

//// aquarium variables init ///

#define CLRUPDT 0x52 // Command to Read Change Color
byte color[4] = {0};

//// aquarium variables end ///


byte slaveAddress = 0;
byte blockRXSize = 0;
bool newKey = false, newByte = false;
char key = '\0';


void setup() {
  pinMode(SLAVE_RESET_PIN, OUTPUT); 
  Wire.begin(4,5); 

  Serial.begin(115200);

  //digitalWrite(SLAVE_RESET_PIN, LOW); // Reset the slave
  //delay(10);
  //digitalWrite(SLAVE_RESET_PIN, HIGH);
  delay(1000);
  while (slaveAddress == 0) {
    slaveAddress = ScanI2C();
    delay(1000);
  }
  clrscr();
  Serial.println("I2C Commands Test");
  Serial.println("=================");
  Serial.println("Please type a command ('a', 's' or 'c'):");
}



void loop() {
  if (newKey == true) {
    newKey = false;
    Serial.println("");
    switch (key) {
      case 'a': case 'A': {
        byte cmdTX[1] = { STDPB1_1 };
        byte txSize = sizeof(cmdTX);
        Serial.print("ESP8266 - Sending Opcode >>> ");
        Serial.print(cmdTX[0]);
        Serial.println("(STDPB1_1)");
        byte transmitData[1] = { 0 };
        for (int i = 0; i < txSize; i++) {
          transmitData[i] = cmdTX[i];
          Wire.beginTransmission(slaveAddress);
          Wire.write(transmitData[i]);
          Wire.endTransmission();
        }
        break;
      }
      case 's': case 'S': {
        byte cmdTX[1] = { STDPB1_0 };
        byte txSize = sizeof(cmdTX);
        Serial.print("ESP8266 - Sending Opcode >>> ");
        Serial.print(cmdTX[0]);
        Serial.println("(STDPB1_0)");
        byte transmitData[1] = { 0 };
        for (int i = 0; i < txSize; i++) {
          transmitData[i] = cmdTX[i];
          Wire.beginTransmission(slaveAddress);
          Wire.write(transmitData[i]);
          Wire.endTransmission();
        }
        break;
      }
      case 'c': case 'C': {
        byte cmdTX[6] = { CLRUPDT, 0, 0, 0, 0, 0};
        byte txSize = sizeof(cmdTX);
        for (int i=0 ; i<4 ; i++){
          newByte = false;
          Serial.print("Please enter a value between 0 and 255 for this command: ");
          while (newByte == false) {
            color[i] = ReadByte();
          }
          Serial.println();
        }        
        if (newByte == true) {
          cmdTX[1] = color[0];
          cmdTX[2] = color[1];
          cmdTX[3] = color[2];
          cmdTX[4] = color[3];
          Serial.print("ESP8266 - Sending Opcode >>> ");
          Serial.print(cmdTX[0]);
          Serial.println("(CLRUPDT)");
          // Transmit command
          byte transmitData[1] = { 0 };
          for (int i = 0; i < txSize; i++) {
            if (i > 0) {
              if (i < 5) {
                Serial.print("ESP8266 - Sending Operand >>> ");
                Serial.println(cmdTX[i]);
              }
              else {
                Serial.print("ESP8266 - Sending CRC >>> ");
                Serial.println(cmdTX[i]);
              }
            }
            transmitData[i] = cmdTX[i];
            Wire.beginTransmission(slaveAddress);
            Wire.write(transmitData[i]);
            Wire.endTransmission();
          }
        }
        break;
      }
      default: {
        Serial.print("ESP8266 - Command '");
        Serial.print(key);
        Serial.println("' unknown ...");
        break;
      }
    }
    Wire.requestFrom(slaveAddress, (byte)1);
    clrscr();
    Serial.println("Please type a command ('a', 's' or 'c'):");
  }
  ReadChar();     
}

// Function ScanI2C
byte ScanI2C() {
  Serial.println("Scanning I2C bus ...");
  byte slaveAddr = 0, scanAddr = 8;
  while (scanAddr < 120) {
    Wire.beginTransmission(scanAddr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found ATtiny85 at address: ");
      Serial.print(scanAddr, DEC);
      Serial.print(" (0x");
      Serial.print(scanAddr, HEX);
      Serial.println(")");
      delay(1000);
      slaveAddr = scanAddr;
    }
    scanAddr++;
  }
  return slaveAddr;
}


void ReadChar() {
  if (Serial.available() > 0) {
    key = Serial.read();
    newKey = true;
  }
}

// Function ReadByte
byte ReadByte() {
  const byte dataLength = 16;
  char serialData[dataLength]; // an array to store the received data  
  static byte ix = 0;
  char rc, endMarker = 0xD; //standard is: char endMarker = '\n'
  while (Serial.available() > 0 && newByte == false) {
    rc = Serial.read();
    if (rc != endMarker) {
      serialData[ix] = rc;
      Serial.print(serialData[ix]);
      ix++;
      if (ix >= dataLength) {
        ix = dataLength - 1;
      }
    }
    else {
      serialData[ix] = '\0'; // terminate the string
      ix = 0;
      newByte = true;
    }
  }
  if ((atoi(serialData) < 0 || atoi(serialData) > 255) && newByte == true) {
    Serial.println("");
    Serial.print("WARNING! Byte values must be 0 to 255 -> Truncating to ");
    Serial.println((byte)atoi(serialData));
  }
  return((byte)atoi(serialData));
}

// Function Clear Screen
void clrscr() {
  Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command
  Serial.write(27);       // ESC command
  Serial.print("[H");     // cursor to home command
}