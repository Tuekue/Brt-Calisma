
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const int digitPins[3] = {8, 12, 9};
int digitScan = 0;
int ones, tens, hundreds;


#define minuteLATCH 4
#define minuteCLK 3
#define minuteDATA 2

#define secondLATCH 7
#define secondCLK 6
#define secondDATA 5

#define heatLATCH 10
#define heatCLK 13
#define heatDATA 11

#define heatDigitHundred 8
#define heatDigitTen 12
#define heatDigitOne 9


//This is the hex value of each number stored in an array by index num
//byte digitOne[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

byte digitOne[10] = {
  B00111111, //0
  B00000110, //1
  B01011011, //2
  B01001111, //3
  B01100110, //4
  B01101101, //5
  B01111101, //6
  B00000111, //7
  B01111111, //8
  B01101111  //9
};

int digitBuffer[3] = {0};
int minute;
int second;
boolean start;

//heat sensor start
int targetHeat = 80;
int targetDelta = 2; // Hedef sicaklik icin +/- aralık

int startStopPin = A2;
int plusPin = A1;
int minusPin = A0;

int valup = 0;
int valdown = 0;
int prevvalup = 0;
int prevvaldown = 0;
unsigned long lastBtnUp = 0;
unsigned long lastBtnDwn = 0;
int transInt = 30;  // Buton basma bekleme süresi. Hızlı basmayı engellemek için.

//heat sensor end

static int startStopPinState = 0;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 500;    // the debounce time; increase if the output flickers


int lastStartState = LOW;
boolean isRunning = false;
unsigned long lastUpdate = 0; // prev tick for seconds
unsigned long minuslastUpdate = 0; 
unsigned long pluslastUpdate = 0; 
unsigned long updateInterval = 1000;  //1000 milisecond -> 1 second

void calcDigits(int num)
{
  ones = num % 10;
  if (num < 10) {
    tens = 0;
  }
  else {
    tens = num / 10;
    hundreds = num / 100;
  }
  return;
}
void displayHeatDigits()
{

  for (byte j = 0; j < 3; j++)
    digitalWrite(digitPins[j], LOW);

  digitalWrite(heatLATCH, LOW);
  shiftOut(heatDATA, heatCLK, MSBFIRST, B11111111);
  digitalWrite(heatLATCH, HIGH);

  delayMicroseconds(100);
  digitalWrite(digitPins[digitScan], HIGH);

  digitalWrite(heatLATCH, LOW);

  shiftOut(heatDATA, heatCLK, MSBFIRST, ~digitOne[digitBuffer[digitScan]]);

  digitalWrite(heatLATCH, HIGH);
  digitScan++;
  if (digitScan > 2) digitScan = 0;

  //  digitalWrite(heatLATCH, LOW);
  //  digitalWrite(heatDigitOne, HIGH);
  //  shiftOut(heatDATA, heatCLK, MSBFIRST, ~digitOne[ones]); // digitRight
  //  digitalWrite(heatLATCH, HIGH);
  //  //delayMicroseconds(100);
  //
  //  digitalWrite(heatLATCH, LOW);
  //  digitalWrite(heatDigitTen, HIGH);
  //  shiftOut(heatDATA, heatCLK, MSBFIRST, ~digitOne[tens]); //
  //  digitalWrite(heatLATCH, HIGH);
  //  //delayMicroseconds(100);
  //
  //  digitalWrite(heatLATCH, LOW);
  //  digitalWrite(heatDigitHundred, HIGH);
  //  shiftOut(heatDATA, heatCLK, MSBFIRST, ~digitOne[hundreds]); // digitLeft
  //  digitalWrite(heatLATCH, HIGH);
}

void setHeater () {
  valup = digitalRead(plusPin);
  valdown = digitalRead(minusPin);

  if (!isRunning) 
  {
    if (valup  != prevvalup && (millis() - lastBtnUp > transInt))
      {
        // increase heat
        targetHeat++;
        calcDigits(targetHeat);
        digitBuffer[2] = hundreds;
        digitBuffer[1] = tens;
        digitBuffer[0] = ones;
        displayHeatDigits();
        //sevseg.write(targetHeat);
      }
      lastBtnUp = millis();
    prevvalup = valup;
    
    if (valdown  != prevvaldown && (millis() - lastBtnDwn > transInt))
      {
        // decrease heat
        targetHeat--;
        calcDigits(targetHeat);
        digitBuffer[2] = hundreds;
        digitBuffer[1] = tens;
        digitBuffer[0] = ones;
        displayHeatDigits();
        //      sevseg.write(targetHeat);
      }
      lastBtnDwn = millis();   
      prevvaldown = valdown;
    }
} //setHeater

void heaterFunction () {

  static int mlxReading = 0;

  if (isRunning == true) {
    //Serial.println("Runninggg");
    mlxReading = mlx.readObjectTempC();

    calcDigits(mlxReading);
    digitBuffer[2] = ones;
    digitBuffer[1] = tens;
    digitBuffer[0] = hundreds;

    displayHeatDigits();
    //sevseg.write(mlxReading);

//    if (mlxReading - targetDelta < targetHeat) {
//      //Relay çekilsin heater açılsın
//
//    }
//    if (mlxReading + targetDelta > targetHeat ) {
//      // Relay bırakılsın heater kapansın
//
//    }

  } else {
    // targetHeat = 80;
    setHeater();
    calcDigits(targetHeat);
    digitBuffer[2] = hundreds;
    digitBuffer[1] = tens;
    digitBuffer[0] = ones;
    displayHeatDigits();

    //    sevseg.write(targetHeat);
  }
} //heaterFunction


void countDown ()
{
  int num;
  calcDigits(minute);
  displayMinuteDigits(ones, tens);

  calcDigits(second);
  displaySecondDigits(ones, tens);
}

void displaySecondDigits(int one, int ten)
{
  digitalWrite(secondLATCH, LOW);
  shiftOut(secondDATA, secondCLK, MSBFIRST, ~digitOne[one]); // digitRight
  shiftOut(secondDATA, secondCLK, MSBFIRST, ~digitOne[ten]); // digitLeft
  digitalWrite(secondLATCH, HIGH);

}

void displayMinuteDigits(int one, int ten)
{
  digitalWrite(minuteLATCH, LOW);
  shiftOut(minuteDATA, minuteCLK, MSBFIRST, ~digitOne[one]); // digitRight
  shiftOut(minuteDATA, minuteCLK, MSBFIRST, ~digitOne[ten]); // digitLeft
  digitalWrite(minuteLATCH, HIGH);

}

void setup() {
  Serial.begin(9600);

  pinMode(minuteLATCH, OUTPUT);
  pinMode(minuteCLK, OUTPUT);
  pinMode(minuteDATA, OUTPUT);

  pinMode(secondLATCH, OUTPUT);
  pinMode(secondCLK, OUTPUT);
  pinMode(secondDATA, OUTPUT);

  pinMode(heatLATCH, OUTPUT);
  pinMode(heatCLK, OUTPUT);
  pinMode(heatDATA, OUTPUT);
  for (int i = 0; i < 3; i++)
  {
    pinMode(digitPins[i], OUTPUT);
  }

  pinMode(startStopPin, INPUT_PULLUP);
  pinMode(plusPin, INPUT_PULLUP);
  pinMode(minusPin, INPUT_PULLUP);

  minute = 05;
  second = 00;
  start = false;
  countDown();

  mlx.begin();
  Serial.println("setup complete");
}

void loop()
{

/* test için.daha sonra kapat
  //isRunning = true; // test için.daha sonra kapat
  //startStopPinState = HIGH; //
*/
int reading = digitalRead(startStopPin);
if (startStopPinState != lastStartState && (millis() - lastDebounceTime) > debounceDelay)
{
  lastDebounceTime = millis();
  startStopPinState = reading;  
  if(startStopPinState == HIGH)
  {
    if (isRunning)
    {
      // Stop basildi
      isRunning = false;
      minute=05;
      second=00;
      countDown();
    }
    else
    {
      // start basildi
      isRunning = true;
    }
  }
 }
lastStartState = reading;
heaterFunction();

if (isRunning == true && millis() - lastUpdate > updateInterval)
  {
    lastUpdate = millis();
    if (second <= 0 )
    {
      minute-- ;
      second = 59;
      if (minute < 0 )
      {
        minute = 10;
        second = 00;
        start = false;
      }
    }
    countDown();
    second--;
  }
  
  if (isRunning == false)
  {
     //digitalRead(plusPin);
     Serial.println(digitalRead(minusPin));
     if (digitalRead(plusPin) == HIGH && millis() - pluslastUpdate > updateInterval)
     {
       pluslastUpdate = millis();       
       minute++;
       
     }
       //digitalRead(minusPin);
     if (digitalRead(minusPin) == LOW && millis() - minuslastUpdate > updateInterval)
     {
      minuslastUpdate = millis();
      minute--;
     }
     countDown();
  }
}

