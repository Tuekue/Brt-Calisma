/*
   created by Rui Santos, http://randomnerdtutorials.com
   Temperature Sensor Displayed on 4 Digit 7 segment common anode
   2013
*/
const int digitPins[3] = {8, 12, 9};                 //3 common anode pins of the display
const int clockPin = 13;    //74HC595 Pin 11
const int latchPin = 10;    //74HC595 Pin 12
const int dataPin = 11;     //74HC595 Pin 14
const byte digit[10] =      //seven segment digits in bits
{
  B00111111, //0
  B00000110, //1
  B01011011, //2
  B01001111, //3
  B01100110, //4
  B01101101, //5
  B01111101, //6
  B00000111, //7
  B01111111, //8
  B01101111  //9};
};
int digitBuffer[3] = {0};
int digitScan = 0, flag = 0,  soft_scaler = 0;
;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 3; i++)
  {
    pinMode(digitPins[i], OUTPUT);
  }
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

//writes the temperature on display
void updateDisp() {
  for (byte j = 0; j < 3; j++)
    digitalWrite(digitPins[j], LOW);

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, B11111111);
  digitalWrite(latchPin, HIGH);

  delayMicroseconds(100);
  digitalWrite(digitPins[digitScan], HIGH);

  digitalWrite(latchPin, LOW);

  shiftOut(dataPin, clockPin, MSBFIRST, ~digit[digitBuffer[digitScan]]);

  digitalWrite(latchPin, HIGH);
  digitScan++;
  if (digitScan > 2) digitScan = 0;
}

void loop() {
  //digitBuffer[3] = int(tempC) / 1000;
  digitBuffer[2] = 1;
  digitBuffer[1] = 2;
  digitBuffer[0] = 8;

  //Serial.println(digitBuffer[3]);
  updateDisp();
  // delay(200);

}
