
#include <SPI.h>

// set up names for the 14 MAX7219 registers:
const byte MAX7219_REG_NOOP        = 0x00;
// codes 1 to 8 are digit positions 1 to 8, see below
const byte MAX7219_REG_DECODEMODE  = 0x09;
const byte MAX7219_REG_INTENSITY   = 0x0A;
const byte MAX7219_REG_SCANLIMIT   = 0x0B;
const byte MAX7219_REG_SHUTDOWN    = 0x0C;
// registers 0x0D and 0x0E are not used
const byte MAX7219_REG_DISPLAYTEST = 0x0F;
// you have 3 digits, so
const byte MAX7219_DIG0        = 0x01;
const byte MAX7219_DIG1        = 0x02;
const byte MAX7219_DIG2        = 0x03;

// 0x00,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F are same as 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15

// and 3 variables to hold data - you will change these later to have different things displayed
byte digit0 = 3;
byte digit1 = 6;
byte digit2 = 9;

byte latchPin = 10; // or whatever pin you used
// 13 SCK will connect to MAX7219 clock
// 11 MOSI will connect to MAX7219 serial data in

// now put some data in the MAX7219 registers, and setup the latchPin - backing off from the function calls

void setup () 
  {
Serial.begin(9600); // setup serial communications for debugging
pinMode (latchPin, OUTPUT);
digitalWrite (latchPin, HIGH);

// turn on SPI library - sets  up the internal hardware for SCK, MISO, MOSI
SPI.begin (); // transfers to the MAX7219 will use default speed of 4 MHz

// now setup the 5 registers that control things
digitalWrite (latchPin, LOW);
SPI.transfer (MAX7219_REG_SCANLIMIT);      // send address
SPI.transfer (2); // send data to show 3 digits
digitalWrite (latchPin,  HIGH); // data latched on this signal going Low to High

//continue for the other registers
digitalWrite (latchPin, LOW);
SPI.transfer (MAX7219_REG_DECODEMODE);      // send address
SPI.transfer (0xFF); // use internal mapping to create the digits
digitalWrite (latchPin,  HIGH); // data latched on this signal going Low to High

//continue for the other registers
digitalWrite (latchPin, LOW);
SPI.transfer (MAX7219_REG_DISPLAYTEST);      // send address
SPI.transfer (0); // no display test (display test on turns on all segments)
digitalWrite (latchPin,  HIGH); // data latched on this signal going Low to High

//continue for the other registers
digitalWrite (latchPin, LOW);
SPI.transfer (MAX7219_REG_INTENSITY);      // send address
SPI.transfer (7); // mid level intensity, 0 to 15
digitalWrite (latchPin,  HIGH); // data latched on this signal going Low to High

//continue for the other registers
digitalWrite (latchPin, LOW);
SPI.transfer (MAX7219_REG_SHUTDOWN);      // send address
SPI.transfer (1); // 1 = not shutdown mode
digitalWrite (latchPin,  HIGH); // data latched on this signal going Low to High

// MAX7219 control registers all set, send some data!
Serial.println ("MAX7219 Setup done");

//continue for the data registers
digitalWrite (latchPin, LOW);
SPI.transfer (MAX7219_DIG0);      // send address
SPI.transfer (digit0); // data
digitalWrite (latchPin,  HIGH); // data latched on this signal going Low to High

//continue for the data registers
digitalWrite (latchPin, LOW);
SPI.transfer (MAX7219_DIG1);      // send address
SPI.transfer (digit1); // 1 = not shutdown mode
digitalWrite (latchPin,  HIGH); // data latched on this signal going Low to High

//continue for the other registers
digitalWrite (latchPin, LOW);
SPI.transfer (MAX7219_DIG2);      // send address
SPI.transfer (digit2); // 1 = not shutdown mode
digitalWrite (latchPin,  HIGH); // data latched on this signal going Low to High

Serial.println ("End of Setup");
}   // end of setup
void loop(){
}
