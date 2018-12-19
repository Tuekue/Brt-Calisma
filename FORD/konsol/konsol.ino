/*

  Cihazlar arası haberleşme için kullanılacak komut seti
  'CMD 1' start process
  'CMD 0' stop process
  'LMU 1' lower lamp
  'LMU 0' stop lowering lamp
  'LMD 1' raise lamp
  'LMD 0 stop rising lamp
  'DST XXX' Distance set to/read XXX (0-999 mm)
  'TIM XXX' Lamp Working time set/remaining

*/
#include "Arduino.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
// #include <openGLCD.h>    // openGLCD library

#include <I2C_graphical_LCD_display.h>
#include <Bounce2.h>

#define LCD_SS 2            //LCD SS
#define StartPin 3          //Start Butonu
#define StopPin 4           //Stop Butonu
#define TimeUpPin 5         //Zaman Artırma Butonu
#define TimeDownPin 6       //Zaman Azaltma Butonu
#define DistanceUpPin 7     //Mesafe Artırma Butonu
#define DistanceDownPin 8   //Mesafe Azaltma Butonu
#define RLiftDown A0        //Alçaltma rölesi
#define RLiftUp A1          //Yükseltme rölesi
#define heatUpPin A2        //Sıcaklık Artırma Butonu
#define heatDownPin A3      //Sıcaklık Azaltma Butonu
#define CE 9                //RF24 CE
#define CSN 10              //RF24 CSN
#define MOSI 11             //RF24 MOSI
#define MISO 12             //RF24 MISO
#define SCK 13              //RF24 SCK

RF24 radio(CE, CSN);

const byte addresses[][6] = {"00001", "00002"}; //RF modül adresleri

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 10;    // the debounce time in ms. increase if the output flickers
Bounce TimeUpDebouncer = Bounce();
Bounce TimeDownDebouncer = Bounce();
Bounce startDebouncer = Bounce();
Bounce stopDebouncer = Bounce();
Bounce DistUpDebouncer = Bounce();
Bounce DistDownDebouncer = Bounce();
Bounce heatUpDebouncer = Bounce();
Bounce heatDownDebouncer = Bounce();

int startReading;
int stopReading;
int timeUpReading;
int timeDownReading;
int distUpReading;
int distDownReading;
int heatDownReading;
int heatUpReading;

uint8_t RadioId;
boolean Start;          //0 Stop, 1 Start
uint8_t LampPosition;   //1 down, 0  up
uint8_t Distance;       // mm cinsinden
uint8_t LCDDistance;    // cm cinsinden
uint8_t sifirPozisyonu;
uint8_t TimeInterval;   //Working time saniye
uint8_t LCDTimeInterval;//Working time dakika
uint8_t sicaklik;

uint8_t minute;
uint8_t second;
boolean startTimer;

char msg[31];
String message;    // Can hold a 30 character string + the null terminator.

boolean isWorking;

// GLCD variables
//gText  textTop = gText(textAreaTOP); // create a text area covering the top half of the display

I2C_graphical_LCD_display lcd;


void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  pinMode(StartPin, INPUT_PULLUP);
  pinMode(StopPin, INPUT_PULLUP);
  pinMode(TimeUpPin, INPUT_PULLUP);
  pinMode(TimeDownPin, INPUT_PULLUP);
  pinMode(DistanceUpPin, INPUT_PULLUP);
  pinMode(DistanceDownPin, INPUT_PULLUP);
  pinMode(heatUpPin, INPUT_PULLUP);
  pinMode(heatDownPin, INPUT_PULLUP);

  pinMode(13, OUTPUT);
  pinMode(RLiftUp, OUTPUT);
  pinMode(RLiftDown, OUTPUT);

  /////////////////////////////////////////
  // Tuslar ve bekleme süreleri ayarlaniyor
  startDebouncer.attach(StartPin);
  startDebouncer.interval(debounceDelay);
  stopDebouncer.attach(StopPin);
  stopDebouncer.interval(debounceDelay);

  TimeUpDebouncer.attach(TimeDownPin);
  TimeUpDebouncer.interval(debounceDelay);
  TimeDownDebouncer.attach(DistanceUpPin);
  TimeDownDebouncer.interval(debounceDelay);

  DistUpDebouncer.attach(TimeUpPin);
  DistUpDebouncer.interval(debounceDelay);
  DistDownDebouncer.attach(DistanceDownPin);
  DistDownDebouncer.interval(debounceDelay);

  heatUpDebouncer.attach(DistanceDownPin);
  heatUpDebouncer.interval(debounceDelay);
  heatDownDebouncer.attach(DistanceDownPin);
  heatDownDebouncer.interval(debounceDelay);

  //default values
  isWorking = false;
  startTimer = false;
  Start = 0; //0 Stop, 1 Start
  LampPosition = 0; //1 down, 0 up
  Distance = 600; //in mm
  LCDDistance = 60; //in cm to show on LCD
  sifirPozisyonu = 400; // in mm
  TimeInterval = 240; //Working time 4min default
  LCDTimeInterval = 4; //Working time 4min default to show on LCD
  sicaklik = 80; //80 derece

  minute = TimeInterval;
  second = 0;
  /* LCD için aşağıyı açmak lazım
    GLCD.Init(); // initialize the display
    textTop.SelectFont(Callibri11); // select the system font for the text area name textTop
    textTop.println("www.brsservis.com"); // print a line of text to the text area.
    GLCD.SelectFont(lcdnums12x16);  // LCD looking font
  */


  lcd.begin ();
  TWBR = 12;
  lcd.gotoxy(0, 0);
  lcd.string("www.brsservis.com");

  radio.begin();
  radio.openWritingPipe(addresses[1]); // 00001
  radio.openReadingPipe(1, addresses[0]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  radio.maskIRQ(1, 1, 0); //Mask all interrupts except the receive interrupt tx ok, tx fail, rx ready

  attachInterrupt(0, radioInterrupt, FALLING);

  /* Saniyede 1 çalışacak kesme ayarlanıyor */
  cli();
  /* Ayarlamaların yapılabilmesi için öncelikle kesmeler durduruldu */

  /* Timer1 kesmesi saniyede bir çalışacak şekilde ayarlanacaktır (1 Hz)*/
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 15624;
  /* Bir saniye aralıklar için zaman sayıcısı ayarlandı */
  TCCR1B |= (1 << WGM12);
  /* Adımlar arasında geçen süre kristal hızının 1024'e bölümü olarak ayarlandı */
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  /* Timer1 kesmesi aktif hale getirildi */

  sei();
  /* Timer1 kesmesinin çalışabilmesi için tüm kesmeler aktif hale getirildi */
  Serial.println("Setup End");
}

/* Arduino otomatik olarak her saniye aşağıdaki fonksiyonu çalıştıracaktır */
ISR(TIMER1_COMPA_vect) {
  char buf[10];

  // Eğer çalışıyorsa süreyi azalt
  if (isWorking) {
    if (second > 0) {
      second--;
    } else {
      minute-- ;
      second = 59;
      //Eğer süre bitti ise işlemi durdur süreyi sıfırla.
      if (minute < 0 )
      {
        isWorking = false;
        minute = TimeInterval;
        second = 0;
      }
    }
    printValues2LCD();
    /*
    snprintf(buf, sizeof(buf), "%02d:%02d", minute, second);
    // draw the formatted string on the display
    lcd.gotoxy (0, 40);
    lcd.string(buf);
    */
  }
}

void loop()
{
  TimeUpDebouncer.update();
  timeUpReading = TimeUpDebouncer.fell();
  TimeDownDebouncer.update();
  timeDownReading = TimeDownDebouncer.fell();
  startDebouncer.update();
  startReading = startDebouncer.fell();
  stopDebouncer.update();
  stopReading = stopDebouncer.fell();
  DistUpDebouncer.update();
  distUpReading = DistUpDebouncer.fell();
  DistDownDebouncer.update();
  distDownReading = DistDownDebouncer.fell();
  heatDownDebouncer.update();
  heatDownReading = heatDownDebouncer.fell();
  heatUpDebouncer.update();
  heatUpReading = heatDownDebouncer.fell();

  processButtons();

}

void radioInterrupt() {

  boolean txOk, txFail, rxReady;
  radio.whatHappened(txOk, txFail, rxReady);
  //Serial.print("Interrupted"); Serial.print(txOk); Serial.print( txFail); Serial.println( rxReady);
  // Serial.print("Aha radyoda program başladı.");Serial.print(txOk);Serial.print( txFail);Serial.print( rxReady);
  if (rxReady)
  {

    //  radio.startListening();
    //  if (radio.available()) {
    //    while (radio.available()) {
    radio.read(&message, sizeof(message));
    Serial.println(message);
  }

  if (message.startsWith("LMD")) {
    digitalWrite(RLiftUp, LOW);
    if (message.substring(4, 1) == "1") {
      // LMP mesajı 1 geldi. İndirme işemini başlat
      digitalWrite(RLiftDown, HIGH);
    }
    if (message.substring(4, 1) == "0") {
      // LMP mesajı 0 geldi. İndirme işemini durdur.
      digitalWrite(RLiftDown, LOW);
    }
  }
  if (message.startsWith("LMPU")) {
    digitalWrite(RLiftDown, LOW);
    if (message.substring(4, 1) == "1") {
      // LMU mesajı 1 geldi. Kaldırma işlemine başla
      digitalWrite(RLiftUp, HIGH);
    }
    if (message.substring(4, 1) == "0") {
      // LMU mesajı 0 geldi. Kaldırma işlemini durdur
      digitalWrite(RLiftUp, LOW);
    }
  }
  //  radio.stopListening();
  //  strncpy( msg, "Message", sizeof(msg) );
  //  radio.write(&msg, sizeof(msg));

  // Gelenmesajları LCD üzerinde göster
  /*


    processMessage(message);




  */
}


void processButtons() {
  if (timeUpReading == HIGH) {
    LCDTimeInterval++;
    minute = LCDTimeInterval;
    second = 0;
    TimeInterval = LCDTimeInterval * 60; //Saniyeye çevir
    Serial.println("TimeInterval : " + TimeInterval);
  }
  if (timeDownReading == HIGH) {
    if (LCDTimeInterval > 0) {
      LCDTimeInterval--;
      minute = LCDTimeInterval;
      second = 0;
      TimeInterval = LCDTimeInterval * 60; //Saniyeye çevir
      Serial.println("TimeInterval : " + TimeInterval);
    }
  }
  if (distUpReading == HIGH) {
    LCDDistance++;
    Distance = LCDDistance * 10; //mm çevir
    Serial.println("Distance : " + Distance);
  }
  if (distDownReading == HIGH) {
    if (LCDDistance > 0) {
      LCDDistance--;
    }
    Distance = LCDDistance * 10; //mm çevir
    Serial.println("Distance : " + Distance);
  }
  if (heatDownReading == HIGH)
  {
    sicaklik --;
    Serial.println("Sicaklik : " + sicaklik);
  }
  if (heatUpReading == HIGH)
  {
    sicaklik ++;
    Serial.println("Sicaklik : " + sicaklik);
  }

  if (startReading == HIGH) {
    Serial.println("Start Working...." );

    isWorking = true;
    minute = LCDTimeInterval;
    second = 0;
    radio.stopListening();
    message = "TIM" + String(TimeInterval); //TimeInterval saniye cinsinden iletilir.
    //radio.write(&message, sizeof(message));
    delay(5);
    radio.startListening();
    message = "DST" + String(Distance);     // Mesafe cm cinsinden iletilir
    radio.stopListening();
    //radio.write(&message, sizeof(message));
    delay(5);
    radio.startListening();
    /*
        message = "LMP1" + String();
        radio.write(&message, sizeof(message));
        delay(5);
    */
    message = "CMD1";
    radio.stopListening();
    //radio.write(&message, sizeof(message));
    delay(5);
    radio.startListening();
  }
  if (stopReading == HIGH) {
    Serial.println("Stop Working...." );

    isWorking = false;
    radio.stopListening();
    message = "CMD0";
    //radio.write(&message, sizeof(message));
    delay(5);
    radio.startListening();
    message = "TIM" + String(TimeInterval);
    //radio.write(&message, sizeof(message));
    radio.stopListening();
    delay(5);
    radio.startListening();
    message = "DST" + String(Distance);
    radio.stopListening();
    //radio.write(&message, sizeof(message));
    delay(5);
    radio.startListening();
    /*
        message = "LMP0";
        radio.write(&message, sizeof(message));
        delay(5);
    */
  }
}
void printValues2LCD() {
  lcd.begin();  //clear everything ??
  lcd.gotoxy(40, 0);
  char buf[30];
  snprintf(buf, sizeof(buf), "DIST:%02d TEMP:%02d TIME:%02d", LCDDistance, sicaklik, TimeInterval);
  lcd.string(buf);
}
void processMessage(String input) {
  input.toUpperCase();
  char buf[10];

  /* Herşeyi foksiyon yazdırsın

    // format the time in a buffer
    snprintf(buf, sizeof(buf), "%02d:%02d", minute, second);
    // draw the formatted string on the display
    lcd.gotoxy (0, 40);
    lcd.string(buf);

  */


  printValues2LCD();

  // Gelen mesajı işeyip LCD'de göster
}
