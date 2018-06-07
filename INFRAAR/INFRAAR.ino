#include <Bounce2.h>

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

#define relayPin A3
#define startStopPin A2
#define minutePlusPin A1
#define minuteMinusPin A0
#define temperturePlusPin A7
#define tempertureMinusPin A6

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
int defaultTempTarget = 20;
int targetTemperture = 20;
int targetDelta = 2; // Hedef sicaklik icin +/- aralÄ±k

int tempValUp = 0;
int tempValDown = 0;
int prevtempValUp = 0;
int prevtempValDown = 0;
unsigned long tempLastBtnUp = 0;
unsigned long tempLastBtnUDown = 0;

int minuteValUp = 0;
int minuteValDown = 0;
int prevMinuteValUp = 0;
int prevMinuteValDown = 0;
unsigned long minuteLastBtnUp = 0;
unsigned long minuteLastBtnUDown = 0;

int transInt = 30;  // Buton basma bekleme sÃ¼resi. HÄ±zlÄ± basmayÄ± engellemek iÃ§in.

//heat sensor end

static int startStopPinState = 0;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 5;    // the debounce time in ms. increase if the output flickers

int lastStartState = LOW;
boolean isRunning = false;
unsigned long lastUpdate = 0; // prev tick for seconds
unsigned long minuslastUpdate = 0;
unsigned long pluslastUpdate = 0;

unsigned long updateInterval = 1000;  //For UpdateTimer. 1000 milisecond -> 1 second

// Instantiate a Bounce object
Bounce OnOffDebouncer = Bounce();
Bounce tempPlusDebouncer = Bounce();
Bounce tempMinusDebouncer = Bounce();
Bounce minPlusDebouncer = Bounce();
Bounce minMinusDebouncer = Bounce();


//////////////////////////////////////////////////////////////////////////////////////////////////////
// EOF declerations


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

void displayTempDigits()
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
  if (digitScan > 2)
    digitScan = 0;
}
void SetTime()
{
  //ilk olarak sure tuslarini kontrol ediyoruz.
  minPlusDebouncer.update();
  int minPlusRead = minPlusDebouncer.fell();

  if (minPlusRead == HIGH)
  {
    //dakika arttirmaya basildi
    Serial.println("min++");
    minute++;

  }
  minMinusDebouncer.update();
  int minMinusRead = minMinusDebouncer.fell();
  if (minMinusRead == HIGH)
  {
    Serial.println("min--");

    minute--;
  }
  UpdateTimer();
}

void setTemperture () {
  tempMinusDebouncer.update();
  int tempValDown = tempMinusDebouncer.fell();
  if (tempValDown == HIGH)
  {
    //isi azalmaya basildi
    Serial.println(targetTemperture);
    targetTemperture--;
  }
  tempPlusDebouncer.update();
  tempValUp = tempPlusDebouncer.fell();
  if (tempValUp == HIGH)
  {
    //isi arttirmaya basildi
    Serial.println(targetTemperture);
    targetTemperture++;
  }
  calcDigits(targetTemperture);
  digitBuffer[0] = hundreds;
  digitBuffer[1] = tens;
  digitBuffer[2] = ones;
  displayTempDigits();
} //setTemperture

void tempertureReading () {

  static int mlxReading = 0;

  if (isRunning == true) {
    //Serial.println("Runninggg");
    mlxReading = mlx.readObjectTempC();

    calcDigits(mlxReading);
    digitBuffer[2] = ones;
    digitBuffer[1] = tens;
    digitBuffer[0] = hundreds;

    displayTempDigits();
    //sevseg.write(mlxReading);

    if (mlxReading - targetDelta < targetTemperture) {
      //Relay Ã§ekilsin heater aÃ§Ä±lsÄ±n
      digitalWrite(relayPin, HIGH);
      Serial.println("Role cek");
    }
    if (mlxReading + targetDelta > targetTemperture ) {
      // Relay bÄ±rakÄ±lsÄ±n heater kapansÄ±n
      digitalWrite(relayPin, LOW);
      Serial.println("Role birak");
    }
  }
} //tempertureReading

// Dakika ve saniye değerlerini 7 SEG gosterir
void UpdateTimer ()
{
  int num;
  calcDigits(minute);
  displayMinuteDigits(ones, tens);

  calcDigits(second);
  displaySecondDigits(ones, tens);
} //UpdateTimer

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

void StartExecute()
{
  isRunning = true;
}

void StopExecute()
{
  isRunning = false;

  //Sicaklik ayarlarini sifirla
  targetTemperture = defaultTempTarget;
  calcDigits(targetTemperture);
  digitBuffer[2] = hundreds;
  digitBuffer[1] = tens;
  digitBuffer[0] = ones;
  displayTempDigits();
  //Sure ayarlarini sifirla
  minute = 05;
  second = 00;
  UpdateTimer();

  digitalWrite(relayPin, LOW);
}

void ReadStartStopPin()
{
  OnOffDebouncer.update();
  int reading = OnOffDebouncer.fell();
  if ( reading == HIGH ) {

    //Basildi

    if (isRunning == true)
    {
      // Stop basildi
      Serial.println("Stop");
      StopExecute();
    }
    else
    {
      // start basildi
      Serial.println("Start");
      StartExecute();
    }
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(relayPin, OUTPUT);

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
  pinMode(minutePlusPin, INPUT_PULLUP);
  pinMode(minuteMinusPin, INPUT_PULLUP);

  pinMode(tempertureMinusPin, INPUT_PULLUP);
  pinMode(tempertureMinusPin, INPUT_PULLUP);

  /////////////////////////////////////////
  // Tuslar ve bekleme süreleri ayarlaniyor

  OnOffDebouncer.attach(startStopPin);
  OnOffDebouncer.interval(debounceDelay); // interval in ms

  tempPlusDebouncer.attach(temperturePlusPin);
  tempPlusDebouncer.interval(debounceDelay);

  tempMinusDebouncer.attach(tempertureMinusPin);
  tempMinusDebouncer.interval(debounceDelay);

  minPlusDebouncer.attach(minutePlusPin);
  minPlusDebouncer.interval(debounceDelay);

  minMinusDebouncer.attach(minuteMinusPin);
  minMinusDebouncer.interval(debounceDelay);

  // Tuslar ve bekleme süreleri ayarlaniyor
  /////////////////////////////////////////

  //Isi olcer calistirilsin
  mlx.begin();

  StopExecute();
  Serial.println("setup complete");
}

void loop()
{
  ReadStartStopPin();
  //Süre geri sayim islemi
  if (isRunning == true ) {
    if (millis() - lastUpdate > updateInterval)
    {
      lastUpdate = millis();
      if (second <= 0 )
      {
        minute-- ;
        second = 59;
        if (minute < 0 )
        {
          StopExecute();
        }
      }
      UpdateTimer();
      second--;
    }
    tempertureReading();
  }
  if (isRunning == false)
  {
    //Sistem durmus. Degisiklik yapilabilir.
    SetTime();

    setTemperture();
  }
}

