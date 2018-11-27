#include <Bounce2.h>

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

LiquidCrystal_PCF8574 lcd(0x20);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#define relay1Pin 2
#define FAN1 13
#define relay2Pin 11
#define FAN2 12

#define startPin 4
#define stopPin 5
#define minutePlusPin 6
#define minuteMinusPin 7
#define singleDoublePin 8

#define SINGLE_LAMP 9
#define DOUBLE_LAMP 10

int minute;
int second;
boolean start;

String  tekCiftYazi="CIFT KASET ";
String  ustSatir="   BRS SERVIS    ";
String  altSatir;

boolean isRunning = false;
unsigned long lastUpdate = 0; // prev tick for seconds
unsigned long updateInterval = 1000;  //For UpdateTimer. 1000 milisecond -> 1 second

int lambSelection = 2;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 10;    // the debounce time in ms. increase if the output flickers

// Instantiate a Bounce object
Bounce minPlusDebouncer = Bounce();
Bounce minMinusDebouncer = Bounce();
Bounce startDebouncer = Bounce();
Bounce stopDebouncer = Bounce();
Bounce lampDebouncer = Bounce();

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
    UpdateTimer();
  }

  if (minMinusRead == HIGH)
  {
    Serial.println("min--");
    minute--;
    if (minute <= 0) minute = 1;
    UpdateTimer();
  }
}

// Dakika ve saniye değerlerini LCD de gosterir
void UpdateTimer ()
{
  altSatir = tekCiftYazi + minute + ":"+ second;
  if (second == 0)  altSatir = altSatir +"0";
  showDisplay(ustSatir, altSatir);
  
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

  // roleleri kapat
  digitalWrite(relay2Pin, HIGH);
  delay(10);
  digitalWrite(FAN2, HIGH);
  
  
  delay(10);
  digitalWrite(relay1Pin, HIGH);
  delay(10);
  digitalWrite(FAN1, HIGH);
    


}

void ReadStartPin()
{
  if ( startReading == HIGH) //Start Tuş basıldı
  {
    Serial.println("Start/Basildi");
    // Calismiyorsa calismaya basla. Calisiyorsa bir sey yapma
    if (isRunning == false) {
      // Start basildi
      Serial.println("Çalışmaya Başla");
      isRunning = true;
  /*     
      if (lambSelection == 1) {
        Serial.println("Tek lamba ledi yakıldı");        
       digitalWrite(DOUBLE_LAMP, LOW);
        delay(10);
        digitalWrite(SINGLE_LAMP, HIGH);
       
      }
       */
      digitalWrite(relay1Pin, LOW);
      delay(10);
      digitalWrite(FAN1, LOW);
      
      Serial.println(lambSelection);
      if (lambSelection == 2) {
        Serial.println("Röle 2 Acildi");
        digitalWrite(relay2Pin, LOW);
        delay(10);
        digitalWrite(FAN2, LOW);
        /*
         delay(10);
        Serial.println("Çift lamba ledi yakıldı");
        digitalWrite(SINGLE_LAMP, LOW);
         delay(10);
        digitalWrite(DOUBLE_LAMP, HIGH);
        */
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
        tekCiftYazi="TEK KASET " + minute + ":"+ second;
        digitalWrite(SINGLE_LAMP, HIGH);
        delay(10);
        digitalWrite(DOUBLE_LAMP, LOW);
      
      } else {
        lambSelection = 2;
        tekCiftYazi="CIFT KASET " + minute + ":"+ second;
        digitalWrite(DOUBLE_LAMP, HIGH);
        delay(10);
        digitalWrite(SINGLE_LAMP, LOW);
      }
        showDisplay(ustSatir, tekCiftYazi);
    }
  }

}

void showDisplay(String  Line1, String  Line2)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(Line1);
    lcd.setCursor(0, 1);
    lcd.print(Line2);
}

void minutePlusMinus()
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
}

void setup()
{
  int error;

  Serial.begin(115200);
  Serial.println("LCD...");
  Serial.println("Dose: check for LCD");

  // See http://playground.arduino.cc/Main/I2cScanner
  Wire.begin();
  Wire.beginTransmission(0x20);
  error = Wire.endTransmission();
  
  Serial.print("Error: ");
  Serial.print(error);

  if (error == 0) {
    Serial.println(": LCD found.");
  } else {
    Serial.println(": LCD not found.");
  } // if

  lcd.begin(16, 2); // initialize the lcd

  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(FAN1, OUTPUT);
  pinMode(FAN2, OUTPUT);

  digitalWrite(relay1Pin, HIGH);
  delay(10);
  digitalWrite(relay2Pin, HIGH);
  delay(10);
  digitalWrite(FAN1, HIGH);
  delay(10);
  digitalWrite(FAN2, HIGH);

  pinMode(SINGLE_LAMP,OUTPUT);
  pinMode(DOUBLE_LAMP,OUTPUT);

  digitalWrite(SINGLE_LAMP, LOW);
  delay(10);
  digitalWrite(DOUBLE_LAMP, HIGH);

  showDisplay(ustSatir," 0532 376 23 44 ");
  delay(1000);

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

  digitalWrite(DOUBLE_LAMP,HIGH);

  StopExecute();

  Serial.println("Setup complete");
}

void  loop()
{ 

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
  } else {
    //Sistem durmus. Degisiklik yapilabilir.
	minutePlusMinus()
	ReadStartPin();
	ReadStopPin();
	ReadSingleDoublePin();
    SetTime();
  }
}
