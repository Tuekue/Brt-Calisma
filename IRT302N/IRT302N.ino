#include <openGLCD.h>    // openGLCD library   
#include <Bounce2.h>
#include "lamba32x32.h"
#include "nolamba32x32.h"

#define relay1Pin 2
#define relay2Pin 13
#define startPin 0
#define stopPin 1
#define singleDoublePin A5
#define minutePlusPin 12
#define minuteMinusPin 3

int minute;
int second;
boolean start;

boolean isRunning = false;
unsigned long lastUpdate = 0; // prev tick for seconds
unsigned long updateInterval = 1000;  //For UpdateTimer. 1000 milisecond -> 1 second

int lambSelection = 2;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 10;    // the debounce time in ms. increase if the output flickers

// Instantiate a Bounce object
//Bounce OnOffDebouncer = Bounce(); // 2 tus ile yapilacagindan bu kullan?lmadi
Bounce minPlusDebouncer = Bounce();
Bounce minMinusDebouncer = Bounce();
Bounce startDebouncer = Bounce();
Bounce stopDebouncer = Bounce();
Bounce lampDebouncer = Bounce();

// GLCD variables
gText  textTop = gText(textAreaTOP); // create a text area covering the top half of the display

int minPlusRead;
int minMinusRead;
int startReading;
int stopReading;
int lampReading;

//////////////////////////////////////////////////////////////////////////////////////////////////////
// EOF declerations


void SetTime()
{
  //ilk olarak sure tuslarini kontrol ediyoruz.

  if (minPlusRead == HIGH)
  {
    //dakika arttirmaya basildi
    Serial.println("min++");
    minute++;
  }

  if (minMinusRead == HIGH)
  {
    Serial.println("min--");
    minute--;
    if (minute <= 0) minute = 1;
  }
  UpdateTimer();
}

// Dakika ve saniye de?erlerini LCD de gosterir
void UpdateTimer ()
{
  char buf[10];

  // format the time in a buffer
  snprintf(buf, sizeof(buf), "%02d:%02d", minute, second);
  // draw the formatted string on the display
  GLCD.DrawString(buf, 0, 40);

} //UpdateTimer

void StartExecute()
{
  isRunning = true;
}

void StopExecute()
{
  isRunning = false;

  //Sure ayarlarini sifirla
  minute = 15;
  second = 0;
  UpdateTimer();

  // lambalari kapat
  digitalWrite(relay2Pin, LOW);
  delay(10);
  digitalWrite(relay1Pin, LOW);
}

void ReadStartPin()
{

  if ( startReading == HIGH) //Start Tu? bas?ld?
  {
    Serial.println("Start/Basildi");
    // Calismiyorsa calismaya basla. Calisiyorsa bir sey yapma
    if (isRunning == false) {
      // Start basildi
      Serial.println("Start");
      isRunning = true;
      digitalWrite(relay1Pin, HIGH);
      Serial.println(lambSelection);
      if (lambSelection == 2) {
        Serial.println("Röle 2 Acildi");
        digitalWrite(relay2Pin, HIGH);
      }
    }
  }

}

void ReadStopPin()
{
  // Calisiyorsa dur. Calismiyorsa bir sey yapma
  if ( stopReading == HIGH) //Stop Basildi
  {
    Serial.println("Stop/Basildi");
    if (isRunning == true) { //Calisiyor o zaman dur
      // Stop basildi
     Serial.println("Stop");
      StopExecute();
    }
  }
}

void ReadSingleDoublePin()
{
  char buf[10];

  if ( lampReading == HIGH ) {
    //    Serial.println("Tek/Cift basildi. Running kontrol edilecek");
    //Basildi
    // Eger calismiyorsa Tek/Cift lamp sectir
    if (isRunning == false)
    {
      //      Serial.println("Tek/Cift basildi. Running kontrol done.");
      //Serial.println(lambSelection);
      if (lambSelection == 2)
      {
        lambSelection = 1;
        GLCD.DrawBitmap(nolamba32x32, 97, 32);
        GLCD.DrawBitmap(lamba32x32, 97, 0);
      } else {
        lambSelection = 2;
        GLCD.DrawBitmap(lamba32x32, 97, 32);
        GLCD.DrawBitmap(lamba32x32, 97, 0);
      }

    }
  }

}
void setup()
{
  Serial.begin(57600);

  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  digitalWrite(relay1Pin, LOW);
  digitalWrite(relay2Pin, LOW);

  GLCD.Init(); // initialize the display
  textTop.SelectFont(Callibri11); // select the system font for the text area name textTop
  textTop.println("www.brsservis.com"); // print a line of text to the text area.

  GLCD.SelectFont(lcdnums12x16);  // LCD looking font
  //  GLCD.SelectFont(fixednums15x31);// larger font


  pinMode(startPin, INPUT_PULLUP);
  pinMode(stopPin, INPUT_PULLUP);
  pinMode(minutePlusPin, INPUT_PULLUP);
  pinMode(minuteMinusPin, INPUT_PULLUP);
  pinMode(singleDoublePin, INPUT_PULLUP);


  /////////////////////////////////////////
  // Tuslar ve bekleme süreleri ayarlaniyor
  startDebouncer.attach(startPin);
  startDebouncer.interval(debounceDelay);

  stopDebouncer.attach(stopPin);
  stopDebouncer.interval(debounceDelay);

  lampDebouncer.attach(singleDoublePin);
  lampDebouncer.interval(debounceDelay);

  minPlusDebouncer.attach(minutePlusPin);
  minPlusDebouncer.interval(debounceDelay);

  minMinusDebouncer.attach(minuteMinusPin);
  minMinusDebouncer.interval(debounceDelay);

  // Tuslar ve bekleme süreleri ayarlaniyor
  /////////////////////////////////////////

  GLCD.DrawBitmap(lamba32x32, 97, 32);
  GLCD.DrawBitmap(lamba32x32, 97, 0);

  StopExecute();

  Serial.println("setup complete");
}

void  loop()
{

  minPlusDebouncer.update();
  minPlusRead = minPlusDebouncer.fell();

  minMinusDebouncer.update();
  minMinusRead = minMinusDebouncer.fell();

  startDebouncer.update();
  startReading = startDebouncer.fell();

  stopDebouncer.update();
  stopReading = stopDebouncer.fell();

  lampDebouncer.update();
  lampReading = lampDebouncer.fell();


  ReadStartPin();
  ReadStopPin();
  ReadSingleDoublePin();
  //Süre geri sayim islemi
  if (isRunning == true ) {
    if (millis() - lastUpdate > updateInterval)
    {
      lastUpdate = millis();
      second--;
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
    }
  }
  if (isRunning == false)
  {
    //Sistem durmus. Degisiklik yapilabilir.
    SetTime();
  }
}