int heatRead0 = 0;
int heatRead1 = 0;

void setup()
{
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  Serial.begin(9600);

}

void loop()
{
  heatRead0 = analogRead(A0);
  heatRead1 = analogRead(A1);
  Serial.println(-40 + 0.488155 * (analogRead(A0) - 20));
  //Serial.print("A0="); Serial.println(heatRead0);
  //Serial.print("A1="); Serial.println(heatRead1);
  Serial.println(digitalRead(A0));
  Serial.println();

 delay(1000); // Delay a little bit to improve simulation performance
}
