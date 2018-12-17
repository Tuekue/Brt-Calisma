/*
Cihazlar arası haberleşme için kullanılacak komut seti
'CMD 1' start process
'CMD 0' stop process
'LMP 1' lower lamp
'LMP 0' raise lamp
'DST XXX' Distance set to/read XXX (0-999 mm)
'TIM XXX' Lamp Working time set/remaining
*/

#include <SPI.h>
#include <NRFLite.h>
#include <RF24.h>

#define led 4
#define button 5

RF24 radio(7, 8); //CNS, CE

const byte addresses[][6] = {"00001", "00002"};
boolean buttonState = 0;

void setup() {
  pinMode(led, OUTPUT);  //led
  pinMode(button, INPUT);  //buton

  Serial.begin(115200);
  Serial.println("Bu bir verici - Alıcı lazım buna");

  radio.begin();
  radio.OpenWritingPipe(addresses[1]);
  radio.OpenReadingPipe(1, addresses[0]);

  radio.setPALevel(RF24_PA_MIN);
}

void loop() {
  delay 5;

  radio.stopListening();
  const char stext[] = "Selam Kardeş";
  const char rtext[] = "";

  radio.write(&stext, sizeof(stext));
  delay(5);

  radio.startListening();
  while (!radio.available());
  radio.read(&rtext, sizeof(text));
  Serial.println(rtext);

  
  radio.read(&buttonState, sizeof(buttonState));

  if (buttonState == HIGH ) {
    digitalWrite(led, HIGH);
  }
  else {
    digitalWrite(led, LOW);
  }


}
