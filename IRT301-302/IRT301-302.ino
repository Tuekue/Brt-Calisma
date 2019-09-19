#include <Bounce2.h>

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

//LiquidCrystal_PCF8574 lcd(0x20);  // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_PCF8574 lcd(0x3F);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#define ILK_LAMBA_relay 9
#define IKINCI_LAMBA_relay 10

#define startPin 4
#define stopPin 5
#define minutePlusPin 6
#define minuteMinusPin 7
#define singleDoublePin 8

//Ledler. çift kaset 
#define TEK_KASET 11
//Ledler. çift  kaset
#define CIFT_KASET 12

int minute;
int second;
boolean start;

String  tekCiftYazi="CIFT KASET ";
String  ustSatir="   BRS SERVIS    ";
String  altSatir;

boolean isRunning = false;
unsigned long lastUpdate = 0; // prev tick for seconds
unsigned long updateInterval = 1000;  //For UpdateTimer. 1000 milisecond -> 1 second

int lambSelection;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 10;    // the debounce time in ms. increase if the output flickers

// Instantiate a Bounce object
Bounce minPlusDebouncer = Bounce();
Bounce minMinusDebouncer = Bounce();
Bounce startDebouncer = Bounce();
Bounce stopDebouncer = Bounce();
Bounce lambaDebouncer = Bounce();

int minPlusRead;
int minMinusRead;
int startReading;
int stopReading;
int lambaReading;

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
  digitalWrite(IKINCI_LAMBA_relay, HIGH);
  delay(10);
  digitalWrite(ILK_LAMBA_relay, HIGH);
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
       digitalWrite(CIFT_KASET, LOW);
        delay(10);
        digitalWrite(TEK_KASET, HIGH);
       
      }
       */
      digitalWrite(ILK_LAMBA_relay, LOW);
      
      Serial.println(lambSelection);
      if (lambSelection == 2) {
        Serial.println("Röle 2 Acildi");
        digitalWrite(IKINCI_LAMBA_relay, LOW);
        /*
         delay(10);
        Serial.println("Çift lamba ledi yakıldı");
        digitalWrite(TEK_KASET, LOW);
         delay(10);
        digitalWrite(CIFT_KASET, HIGH);
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
  if ( lambaReading == HIGH ) {
    //    Serial.println("Tek/Cift basildi. Running kontrol edilecek");
    //Basildi
    // Eger calismiyorsa Tek/Cift lamba sectir
    if (isRunning == false)
    {
      //      Serial.println("Tek/Cift basildi. Running kontrol done.");
      //Serial.println(lambSelection);
      if (lambSelection == 2)
      {
        //Tek Kaset seçildi
        lambSelection = 1;
        tekCiftYazi = "TEK KASET " + minute ;
        tekCiftYazi = tekCiftYazi + ":" + second;
        digitalWrite(TEK_KASET, HIGH); //Led high iken sönüyorsa değişecek
        delay(10);
        digitalWrite(CIFT_KASET, LOW);
      
      } else {
        //Çift Kaset seçildi
        lambSelection = 2;
        tekCiftYazi = "CIFT KASET " + minute ;
        tekCiftYazi = tekCiftYazi + ":"+ second;
        digitalWrite(CIFT_KASET, HIGH);
        delay(10);
        digitalWrite(TEK_KASET, LOW);
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

  lambaDebouncer.update();
  lambaReading = lambaDebouncer.fell();
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

  pinMode(ILK_LAMBA_relay, OUTPUT);
  pinMode(IKINCI_LAMBA_relay, OUTPUT);
  
  digitalWrite(ILK_LAMBA_relay, HIGH);
  digitalWrite(IKINCI_LAMBA_relay, HIGH);
  
  pinMode(TEK_KASET,OUTPUT);
  pinMode(CIFT_KASET,OUTPUT);
  
  //İlk çalışmada iki kaset default
  lambSelection = 2;
  digitalWrite(TEK_KASET, LOW);
  digitalWrite(CIFT_KASET, HIGH);

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

  lambaDebouncer.attach(singleDoublePin);
  lambaDebouncer.interval(debounceDelay);

  minPlusDebouncer.attach(minutePlusPin);
  minPlusDebouncer.interval(debounceDelay);

  minMinusDebouncer.attach(minuteMinusPin);
  minMinusDebouncer.interval(debounceDelay);

  // Tuslar ve bekleme süreleri ayarlaniyor
  /////////////////////////////////////////

  digitalWrite(CIFT_KASET,HIGH);

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
	minutePlusMinus();
	ReadStartPin();
	ReadStopPin();
	ReadSingleDoublePin();
  SetTime();
  }
}
