/*
MLX90614 ------------- Arduino
 VDD ------------------ 3.3V
 VSS ------------------ GND
 SDA ------------------ SDA (A4)
 SCL ------------------ SCL (A5)
*/
#include <Wire.h> 
#include <SparkFunMLX90614.h> 
  
IRTherm therm; 
const byte LED_PIN = 13; 
  
void setup() 
{
 Serial.begin(9600); 
 therm.begin(); 
 therm.setUnit(TEMP_C); 
 pinMode(LED_PIN, OUTPUT); 
 setLED(LOW); 
}
  
void loop() 
{
 setLED(HIGH);
  
 if (therm.read()) 
 {
 Serial.print("Object: " + String(therm.object(), 2));
 Serial.write('°'); 
 Serial.println("F");
 Serial.print("Ambient: " + String(therm.ambient(), 2));
 Serial.write('°'); 
 Serial.println("F");
 Serial.println();
 }
 setLED(LOW);
 delay(500);
}
  
void setLED(bool on)
{
 if (on)
 digitalWrite(LED_PIN, LOW);
 else
 digitalWrite(LED_PIN, HIGH);
}
