#include <openGLCD.h>    // openGLCD library   
#include <Bounce2.h>
#include "lamp32x32.h"

#define relay1Pin 2
#define relay2Pin 3
#define startPin A6
#define stopPin A7
#define singleDoublePin A5
#define minutePlusPin 12
#define minuteMinusPin 13

int minute;
int second;
boolean start;

int minuteValUp = 0;
int minuteValDown = 0;

int prevMinuteValUp = 0;
int prevMinuteValDown = 0;

unsigned long minuteLastBtnUp = 0;
unsigned long minuteLastBtnUDown = 0;

int transInt = 30;  // Buton basma bekleme sÃ¼resi. HÄ±zlÄ± basmayÄ± engellemek iÃ§in.

static int startPinState = 0;
static int stopPinState = 0;
static int singleDoublePinState = 0;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 5;    // the debounce time in ms. increase if the output flickers

int lastStartState = LOW;
boolean isRunning = false;
int lambSelection = 1;

unsigned long lastUpdate = 0; // prev tick for seconds
unsigned long minuslastUpdate = 0;
unsigned long pluslastUpdate = 0;

unsigned long updateInterval = 1000;  //For UpdateTimer. 1000 milisecond -> 1 second

// Instantiate a Bounce object
Bounce OnOffDebouncer = Bounce();
Bounce SingleDoubleDebouncer = Bounce();
Bounce minPlusDebouncer = Bounce();
Bounce minMinusDebouncer = Bounce();

// GLCD variables
gText  textTop = gText(textAreaTOP); // create a text area covering the top half of the display

//////////////////////////////////////////////////////////////////////////////////////////////////////
// EOF declerations

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

// Dakika ve saniye değerlerini LCD de gosterir
void UpdateTimer ()
{
  char buf[10];

  // format the time in a buffer
  snprintf(buf, sizeof(buf), "%02d:%02d", minute, second);
  // draw the formatted string centered on the display
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
  digitalWrite(relay1Pin, LOW);
  digitalWrite(relay2Pin, LOW);
}

void ReadStartStopPin()
{
  if (digitalRead(startPin) == HIGH) //functions based off of button pulling input pin LOW
  {
    // Calismiyorsa calismaya basla. Calisiyorsa bir sey yapma
    if (isRunning == false) {
      // Start basildi
      Serial.println("Start");
      StartExecute();
    }
  }
  // Calisiyorsa dur. Calismiyorsa bir sey yapma
  if (isRunning == true) {
    if (digitalRead(stopPin) == HIGH) //functions based off of button pulling input pin LOW
    {
      // Stop basildi
      Serial.println("Stop");
      StopExecute();
    }
  }
}

void ReadSingleDoublePin()
{
  char buf[10];

  SingleDoubleDebouncer.update();
  int reading = SingleDoubleDebouncer.fell();
  if ( reading == HIGH ) {
    //Basildi
    // Eger calismiyorsa Tek/Cift lamp sectir
    if (isRunning == false)
    {
      Serial.println("Tek/Cift basildi");
      if (lambSelection == 2)
      {
        lambSelection = 1;
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
  Serial.begin(9600);

  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);

  GLCD.Init(); // initialize the display
  textTop.SelectFont(Callibri11); // select the system font for the text area name textTop
  textTop.println("www.brsservis.com"); // print a line of text to the text area.

  GLCD.SelectFont(lcdnums12x16);  // LCD looking font
  //  GLCD.SelectFont(fixednums15x31);// larger font

  pinMode(startPin, INPUT_PULLUP);
  pinMode(stopPin, INPUT_PULLUP);
  pinMode(minutePlusPin, INPUT_PULLUP);
  pinMode(minuteMinusPin, INPUT_PULLUP);

  /////////////////////////////////////////
  // Tuslar ve bekleme süreleri ayarlaniyor

  minPlusDebouncer.attach(minutePlusPin);
  minPlusDebouncer.interval(debounceDelay);

  minMinusDebouncer.attach(minuteMinusPin);
  minMinusDebouncer.interval(debounceDelay);

  // Tuslar ve bekleme süreleri ayarlaniyor
  /////////////////////////////////////////

      GLCD.DrawBitmap(lamba32x32, 97, 32);
      GLCD.DrawBitmap(lamba32x32, 97, 0);
      
  StopExecute();
  //StartExecute();

  Serial.println("setup complete");

}

void  loop()
{
  ReadStartStopPin();
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
