/*
  Cihazlar arası haberleşme için kullanılacak komut seti
  'CMD 1' start process
  'CMD 0' stop process
  'LMU 1' lower lamp
  'LMU 0' stop lowering lamp
  'LMD 1' raise lamp
  'LMD 0 stop rising lamp
  'DST XXX' Durma mesafesi oku/ayarla XXX (0-999 mm)
  'TIM XXX' Lamba çalışma süresi oku/ayarla (saniye)
  'HEA XXX' Ölçülen Sıcaklık
*/

#include "Arduino.h"
#include <SPI.h>
#include <U8g2lib.h>
#include <SoftwareSerial.h> // soft serial

/*
  void setup();
  void loop();
  void commWithSerial();
  void processButtons(int buton);
  void lcdTimeDisplay();
  void processMessage(String input);
*/
#define rxPin 9 // soft serial
#define txPin 12 // soft serial

#define StartPin 2          //Start Butonu
#define StopPin 3          //Stop Butonu
#define TimeUpPin 4         //Zaman Artırma Butonu
#define TimeDownPin 5       //Zaman Azaltma Butonu
#define DistanceUpPin 6     //Mesafe Artırma Butonu
#define DistanceDownPin 7   //Mesafe Azaltma Butonu
#define RLiftDown A0        //Alçaltma rölesi
#define RLiftUp A1          //Yükseltme rölesi
#define heatUpPin A2        //Sıcaklık Artırma Butonu
#define heatDownPin A3      //Sıcaklık Azaltma Butonu

#define NumOfButtons 8

U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 8);

uint8_t buttons[NumOfButtons] = {TimeUpPin, TimeDownPin, DistanceUpPin, DistanceDownPin, heatUpPin, heatDownPin, StartPin, StopPin};
boolean butgate[NumOfButtons];

boolean Start;          //0 Stop, 1 Start
uint8_t LampPosition;   //1 down, 0  up
uint8_t Distance;       // mm cinsinden
uint8_t LCDDistance;    // cm cinsinden
uint8_t sifirPozisyonu;
uint8_t TimeInterval;   //Working time saniye
uint8_t LCDTimeInterval;//Working time dakika
int sicaklik;
boolean newData = false;

int minute;
int second;
int bekleme;

boolean isWorking;
SoftwareSerial mySerial = SoftwareSerial (rxPin, txPin);
String LCDInfo;

//////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP /////
void setup() {
  Serial.begin(9600);
  Serial.println("Start konsol");

  for (int i = 0; i < NumOfButtons; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    butgate[i] = false;
  }
  pinMode(RLiftUp, OUTPUT);
  digitalWrite(RLiftUp, HIGH);
  pinMode(RLiftDown, OUTPUT);
  digitalWrite(RLiftDown, HIGH);

  pinMode (rxPin, INPUT);
  pinMode (txPin, OUTPUT);
  mySerial.begin(9600);

  //default values
  isWorking = false;
  Start = 0; //0 Stop, 1 Start
  LampPosition = 0; //1 asagi, 0 yukari
  Distance = 600; // mm
  LCDDistance = 60; // cm LCD de gösterim
  sifirPozisyonu = 400; // mm
  LCDTimeInterval = 4; //Calisma suresi 4 dk default LCD de gösterim
  TimeInterval = LCDTimeInterval * 60; // Calisma suresi 240 sn default (4x60)
  sicaklik = 80; //80 derece

  minute = 4; //Calisma suresi 4 dk default
  second = 0;
  bekleme = 0;

  u8g2.begin();
  u8g2.setFontMode(0);
  LCDInfo="";
  printValues2LCD();

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

/////////////////////////////////////////////////////////////////////////////
/* Arduino otomatik olarak her saniye aşağıdaki fonksiyonu çalıştıracaktır */
/* Time interrupt function                                                 */
/////////////////////////////////////////////////////////////////////////////
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
        minute = LCDTimeInterval;
        second = 0;
      }
    }
    if (bekleme > 0 && millis() >= bekleme +  60000) // 1 dk = 60 sn x 1000 ms
    {
      digitalWrite(RLiftUp, HIGH);
      isWorking = false;
      bekleme = 0;
    }
    printValues2LCD();
  }
}

////////////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP /////
void loop()
{
  for (int i = 0; i < NumOfButtons; i++)
  {
    if (!butgate[i])
    {
      if (digitalRead(buttons[i]) == LOW)
      {
        butgate[i] = true;
        processButtons(i);

      }
    }
    if (butgate[i])
    {
      if (digitalRead(buttons[i]) == HIGH)
      {
        butgate[i] = false;
      }
    }
  }
  commWithSerial();
  //printValues2LCD();
}


// Bu fonksiyonun işlevlerini radio olmadan yapalım
// Lamba ile mesajlaşma işleri
void commWithSerial() {

  String message;
  message = "";

  if (mySerial.available() > 0)
  {
    message = mySerial.readString();
    Serial.print("Gelen mesaj: "); Serial.println(message);

    if (message == "<LMD1>") {
      LCDInfo = "Lift indiriliyor";
      digitalWrite(RLiftUp, HIGH);
      delay(3000);
      // LMP mesajı 1 geldi. İndirme işemini başlat
      digitalWrite(RLiftDown, LOW);
    }
    if (message == "<LMD0>") {
      LCDInfo = "Lift durduruldu.";
      digitalWrite(RLiftUp, HIGH);
      delay(3000);
      // LMP mesajı 0 geldi. İndirme işemini durdur.
      digitalWrite(RLiftDown, HIGH);
      isWorking = true;
    }
    if (message == "<LMU1>") {
      LCDInfo = "Lift kaldırılıyor";
      digitalWrite(RLiftDown, HIGH);
      delay(3000);
      // LMU mesajı 1 geldi. Kaldırma işlemine başla
      digitalWrite(RLiftUp, LOW);
      bekleme = millis();
    }
    if (message == "<LMU0>") {
      LCDInfo = "Lift durduruldu.";
      digitalWrite(RLiftDown, HIGH);
      delay(3000);
      // LMU mesajı 0 geldi. Kaldırma işlemini durdur
      digitalWrite(RLiftUp, HIGH);
      isWorking = false;
    }
    Serial.println(LCDInfo);
    printValues2LCD();
  }
}


void processButtons(int buton) {
  String message;

  message = "";
  Serial.print("Buton basıldı"); Serial.println(buton);
  if (!isWorking)
  {
    if (buttons[buton] == TimeUpPin)
    {

      LCDTimeInterval++;
      minute = LCDTimeInterval;
      second = 0;
      TimeInterval = LCDTimeInterval * 60; //Saniyeye çevir
      //Serial.print("TimeInterval : "); Serial.print(TimeInterval); Serial.print(" LCD Interval: "); Serial.println(LCDTimeInterval);
    }
    if  (buttons[buton] == TimeDownPin)
    {

      LCDTimeInterval--;
      minute = LCDTimeInterval;
      second = 0;
      TimeInterval = LCDTimeInterval * 60; //Saniyeye çevir
      //Serial.print("TimeInterval : "); Serial.print(TimeInterval); Serial.print(" LCD Interval: "); Serial.println(LCDTimeInterval);
    }
    if (buttons[buton]  == DistanceUpPin)
    {
      LCDDistance++;
      Distance = LCDDistance * 10; //mm çevir
      //Serial.print("Distance : "); Serial.print(Distance); Serial.print(" LCD Distance : "); Serial.println(LCDDistance);
    }
    if (buttons[buton]  == DistanceDownPin)
    {
      if (LCDDistance > 0) {
        LCDDistance--;
      }
      Distance = LCDDistance * 10; //mm çevir
      //Serial.print("Distance : "); Serial.print(Distance); Serial.print(" LCD Distance : "); Serial.println(LCDDistance);
    }
    if (buttons[buton]  == heatDownPin)
    {
      sicaklik --;
      Serial.print("Sıcaklık - : "); Serial.println(sicaklik);
    }
    if (buttons[buton]  == heatUpPin)
    {
      sicaklik ++;
      Serial.print("Sıcaklık + : "); Serial.println(sicaklik);
    }
    if (buttons[buton]  == StartPin)
    {
      Serial.println("Start basildi...." );
      isWorking = true;
      minute = LCDTimeInterval;
      second = 0;

      // Lambaya mesaj yolla
      message = "<TIM" + String(TimeInterval) + ">"; //TimeInterval saniye cinsinden iletilir.
      mySerial.print(message);
      delay(1000);
      // Lambaya mesaj yolla
      message = "<DST" + String(Distance) + ">";     // Mesafe cm cinsinden iletilir
      mySerial.print("<DST"); Serial.print(Distance); Serial.println(">");
      delay(1000);
      // Lambaya mesaj yolla
      message = "<HEA" + String(sicaklik) + ">";     // Mesafe cm cinsinden iletilir
      mySerial.print("<HEA"); Serial.print(sicaklik); Serial.println(">");
      delay(1000);
      // Lambaya mesaj yolla
      mySerial.print("<CMD1>");

      LCDInfo = "ÇALIŞIYOR";
      delay(1000);
    }
  }
  if (isWorking)
  {
    if (buttons[buton]  == StopPin)
    {
      //Serial.println("Stop basildi...." );
      mySerial.print("<CMD0>");
      delay(1000);
      minute = LCDTimeInterval;
      second = 0;
      isWorking = false;
      LCDInfo = "DURDU";
      // Lambaya mesaj yolla
    }
  }
  printValues2LCD();
}

void printValues2LCD() {

  u8g2.setFont(u8g2_font_mozart_nbp_tr);
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 10);
    u8g2.print(F("MESAFE: "));
    u8g2.print(LCDDistance);

    u8g2.setCursor(0, 20);
    u8g2.print(F("SICAKLIK: "));
    u8g2.print(sicaklik);

    u8g2.setCursor(0, 30);
    u8g2.print(F("ZAMAN: "));
    if (minute < 10) {
      u8g2.print(F("0"));
    }
    u8g2.print(minute);
    u8g2.print(F(":"));

    if (second < 10) {
      u8g2.print(F("0"));
    }
    u8g2.print(second);

    u8g2.setCursor(0, 40);
    u8g2.print(LCDInfo);

    u8g2.setCursor(0, 55);
    u8g2.print(F("www.brsservis.com"));
  } while ( u8g2.nextPage() );

}
