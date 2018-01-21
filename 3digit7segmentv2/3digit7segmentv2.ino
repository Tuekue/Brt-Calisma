/*
  JimboZA April 2016
  uses sevseg library
  the sevseg refresh means NO blocking code may be used, hence elapsedMillis
  change the .h to reflect segment resistors
*/

#include <SevSeg.h>
#include <elapsedMillis.h> //bwod in a box

SevSeg sevseg; //Instantiate a seven segment controller obj
elapsedMillis timeElapsed;
unsigned int interval = 1000; //how many ms each value displays

byte theNum = 0; // what I want to display

void setup() {

  Serial.begin(9600);
  // put your setup code here, to run once:
  byte numDigits = 3;
  byte digitPins[] = {9, 10, 11};
  byte segmentPins[] = {2, 3, 4, 5, 6, 7, 8}; //Segments: A,B,C,D,E,F,G,Period

  sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(100);

  // Jim has resistors on the segments, so edited sevseg.h thus:
  // If you use current-limiting resistors on your segment pins instead of the
  // digit pins, then change the '0' in the line below to a '1'
  //#define RESISTORS_ON_SEGMENTS 1

  Serial.println("setup complete");

}

void loop() {
  // put your main code here, to run repeatedly:

  sevseg.setNumber(theNum, 0);

  sevseg.refreshDisplay(); //must not block this

  //delay(500); // this screws the refresh up so bwod required:

  if (timeElapsed > interval)
  {
    theNum++;
    if (theNum == 1000) theNum = 0;
    timeElapsed = 0;
  }
}//loop
