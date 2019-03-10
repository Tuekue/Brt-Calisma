#define LampRelay 11            //Lamba rölesi

void setup()
{
  pinMode(LampRelay, OUTPUT);
  digitalWrite(LampRelay, HIGH);
  Serial.begin(9600);

}

void loop()
{
  digitalWrite(LampRelay, LOW);
  delay(1000); // Delay a little bit to improve simulation performance
  digitalWrite(LampRelay, HIGH);
  delay(1000); // Delay a little bit to improve simulation performance
}
