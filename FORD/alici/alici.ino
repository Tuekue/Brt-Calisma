
#include <SPI.h>
#include <NRFLite.h>
#include <RF24.h>

RF24 radio(7, 8); //CNS, CE

const byte addresses[][6] = {"00001", "00002"};
boolean buttonState = 0;

void setup() {
  // put your setup code here, to run once:

  pinMode(led, OUTPUT);  //led
  pinMode(button, INPUT);  //buton

  Serial.begin(115200);
  Serial.println("Bu bir alici - Verici lazım buna");

  radio.begin();
  radio.OpenWritingPipe(addresses[1]);
  radio.OpenReadingPipe(1, addresses[0]);

  radio.setPALevel(RF24_PA_MIN);
}

void loop() {

  delay 5;

  radio.startListening();

  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
  }
    radio.stopListening();

}
