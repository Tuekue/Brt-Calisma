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
#include <openGLCD.h>    // openGLCD library   
#include <Bounce2.h>

#define LCD_SS 2            //Star Butonu
#define StartPin 3          //Star Butonu
#define StopPin 4           //Stop Butonu
#define TimeUpPin 5         //Zaman Artırma Butonu
#define TimeDownPin 6       //Zaman Azaltma Butonu
#define DistanceUpPin 7     //Mesafe Artırma Butonu
#define DistanceDownPin 8   //Mesafe Azaltma Butonu
#define RLiftDown A0        //Alçaltma rölesi
#define RLiftUp A1          //Yükseltme rölesi
#define limitswitchUpPin A2     //Mesafe Artırma Butonu
#define limitswitchDownPin A3   //Mesafe Azaltma Butonu
#define CE 9
#define CSN 10
#define MOSI 11
#define MISO 12
#define SCK 13

#define _fanTime 60

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
Bounce limitswitchUpDebouncer = Bounce();
Bounce limitswitchDownDebouncer = Bounce();


int startReading;
int stopReading;
int timeUpReading;
int timeDownReading;
int distUpReading;
int distDownReading;


uint8_t RadioId;
boolean Start;          //0 Stop, 1 Start
uint8_t LampPosition;   //1 down, 0  up
uint8_t Distance;       // mm cinsinden
uint8_t LCDDistance;    // cm cinsinden
uint8_t sifirPozisyonu;
uint8_t TimeInterval;   //Working time saniye
uint8_t LCDTimeInterval;//Working time dakika
uint8_t fanTimer;

uint8_t minute;
uint8_t second;
boolean startTimer;


String message;    // Can hold a 30 character string + the null terminator.
boolean isWorking;

// GLCD variables
gText  textTop = gText(textAreaTOP); // create a text area covering the top half of the display


void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  pinMode(StartPin, INPUT_PULLUP);
  pinMode(StopPin, INPUT_PULLUP);
  pinMode(TimeUpPin, INPUT_PULLUP);
  pinMode(TimeDownPin, INPUT_PULLUP);
  pinMode(DistanceUpPin, INPUT_PULLUP);
  pinMode(DistanceDownPin, INPUT_PULLUP);
  pinMode(limitswitchUpPin, INPUT_PULLUP);
  pinMode(limitswitchDownPin, INPUT_PULLUP);

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

  limitswitchUpDebouncer.attach(DistanceDownPin);
  limitswitchUpDebouncer.interval(debounceDelay);
  limitswitchDownDebouncer.attach(DistanceDownPin);
  limitswitchDownDebouncer.interval(debounceDelay);

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
  fanTimer = _fanTime;

  minute = TimeInterval;
  second = 0;
  /* LCD için aşağıyı açmak lazım
    GLCD.Init(); // initialize the display
    textTop.SelectFont(Callibri11); // select the system font for the text area name textTop
    textTop.println("www.brsservis.com"); // print a line of text to the text area.
    GLCD.SelectFont(lcdnums12x16);  // LCD looking font
  */
  radio.begin();
  radio.openWritingPipe(addresses[1]); // 00001
  radio.openReadingPipe(1, addresses[0]); // 00002
  radio.setPALevel(RF24_PA_MIN);

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

    snprintf(buf, sizeof(buf), "%02d:%02d", minute, second);
    // draw the formatted string on the display
    GLCD.DrawString(buf, 0, 40);
  }
}

void loop()
{
  Serial.println("Rlift HIGH");
  digitalWrite(13, HIGH);
  //digitalWrite(RLiftUp, HIGH);
  delay(100);
  Serial.println("Rlift LOW");
  digitalWrite(13, LOW);
  //digitalWrite(RLiftUp, LOW);
  delay(100);
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


  // Limit switche geldi ise ilgili motoru durdur.
  if (limitswitchDownDebouncer.fell())
  {
    // LiftDown motoru dursun
    digitalWrite(RLiftDown, LOW);
  }
  if (limitswitchUpDebouncer.fell())
  {
    // LiftUp motoru dursun
    digitalWrite(RLiftUp, LOW);
  }

  processButtons();

  radio.startListening();
  if ( radio.available()) {
    while (radio.available()) {
      radio.read(&message, sizeof(message));
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
    }
    delay(5);
    radio.stopListening();
    // Gelenmesajları LCD üzerinde göster
    processMessage(message);
  }
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
  if (startReading == HIGH) {
    Serial.println("Start Working...." );

    isWorking = true;
    minute = LCDTimeInterval;
    second = 0;
    radio.stopListening();
    message = "TIM" + String(TimeInterval); //TimeInterval saniye cinsinden iletilir.
    radio.write(&message, sizeof(message));
    delay(5);
    message = "DST" + String(Distance);     // Mesafe cm cinsinden iletilir
    radio.write(&message, sizeof(message));
    delay(5);
    /*
        message = "LMP1" + String();
        radio.write(&message, sizeof(message));
        delay(5);
    */
    message = "CMD1";
    radio.write(&message, sizeof(message));
    delay(5);
    radio.startListening();
  }
  if (stopReading == HIGH) {
    Serial.println("Stop Working...." );

    isWorking = false;
    radio.stopListening();
    message = "CMD0";
    radio.write(&message, sizeof(message));
    delay(5);
    message = "TIM" + String(TimeInterval);
    radio.write(&message, sizeof(message));
    delay(5);
    message = "DST" + String(Distance);
    radio.write(&message, sizeof(message));
    delay(5);
    /*
        message = "LMP0";
        radio.write(&message, sizeof(message));
        delay(5);
    */
    radio.startListening();
  }
}

void processMessage(String input) {
  input.toUpperCase();
  char buf[10];

  // format the time in a buffer
  snprintf(buf, sizeof(buf), "%02d:%02d", minute, second);
  // draw the formatted string on the display
  GLCD.DrawString(buf, 0, 40);

  // Gelen mesajı işeyip LCD'de göster
}
