#include <SevenSeg.h>

#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

SevenSeg sevseg(2, 3, 4, 5, 6, 7, 8); //Instantiate a seven segment controller obj
const int numOfDigits = 3;
int digitPins[numOfDigits] = {9, 10, 11};

// Set your target heat here.
// TODO: Gerçekte bu değer 60-120 arası olacak
int targetHeat = 30;
int targetDelta = 2; // Hedef sicaklik icin +/- aralık

int startStopPin = 12;
int plusPin = A1;
int minusPin = A0;
boolean isRunning = false;

int valup = 0;
int valdown = 0;
int prevvalup = 0;
int prevvaldown = 0;
unsigned long lastBtnUp = 0;
unsigned long lastBtnDwn = 0;
int transInt = 30;  // Buton basma bekleme süresi. Hızlı basmayı engellemek için.


void setHeater () {
  valup = digitalRead(plusPin);
  valdown = digitalRead(minusPin);

  if (valup == HIGH && prevvalup == LOW) //funcitons based off of button pulling input pin LOW
  {
    if ((!isRunning) && (millis() - lastBtnUp > transInt))
    {
      // increase heat
      targetHeat++;
      sevseg.write(targetHeat);
      Serial.println(targetHeat);
    }
    lastBtnUp = millis();
  }
  prevvalup = valup;

  if (valdown == HIGH && prevvaldown == LOW) //funcitons based off of button pulling input pin LOW
  {
    if ((!isRunning) && (millis() - lastBtnDwn > transInt))
    {
      // decrease heat
      targetHeat--;
      sevseg.write(targetHeat);
      Serial.println(targetHeat);
    }
    lastBtnDwn = millis();
  }
  prevvaldown = valdown;

} //setHeater

void heaterFunction () {

  static int mlxReading = 0;

  if (isRunning) {
    mlxReading = round(mlx.readObjectTempC());
    sevseg.write(mlxReading);

    if (mlxReading - targetDelta < targetHeat) {
      //Relay çekilsin heater açılsın
      digitalWrite(LED_BUILTIN, HIGH);
    }
    if (mlxReading + targetDelta > targetHeat ) {
      // Relay bırakılsın heater kapansın
      digitalWrite(LED_BUILTIN, LOW);
    }

  } else {
    // targetHeat = 30;
    setHeater();
    sevseg.write(targetHeat);
  }
} //heaterFunction

void setup() {

  Serial.begin(9600);
  // put your setup code here, to run once:

  mlx.begin();
  sevseg.setDigitPins(numOfDigits, digitPins);


  // Sözde relay ;) pin 13 yanıyorsa relay çekildi demektir.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(startStopPin, INPUT_PULLUP);
  pinMode(plusPin, INPUT_PULLUP);
  pinMode(minusPin, INPUT_PULLUP);

  Serial.println("setup complete");
} //setup

void loop() {

  static int startStopPinState = 0;
  Serial.println(isRunning);

  startStopPinState = digitalRead(startStopPin);
  //Serial.println(startStopPinState);

  if (startStopPinState == HIGH) //funcitons based off of button pulling input pin HIGH
  {
    if (isRunning)
    {
      // Stop basildi

      digitalWrite(LED_BUILTIN, LOW);
      isRunning = false;
    }
    else
    {
      // start basildi
      isRunning = true;
    }
  }
  heaterFunction();

}//loop
