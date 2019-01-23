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
  'HEA XXX' Ayarlanan Sıcaklık
  'ISI XXX' Ölçülen Sıcaklık
  'LRD1 Lamba ready
  'CRD1 Konsol ready
*/
/*
  #define StartButton 2          //Start Butonu
  #define StopButton 3          //Stop Butonu
  #define TimeUpButton 4         //Zaman Artırma Butonu
  #define TimeDownButton 5       //Zaman Azaltma Butonu
  #define DistanceUpButton 6     //Mesafe Artırma Butonu
  #define DistanceDownButton 7   //Mesafe Azaltma Butonu
  #define heatUpButton A2        //Sıcaklık Artırma Butonu
  #define heatDownButton A3      //Sıcaklık Azaltma Butonu
*/

#include "Arduino.h"
#include <SPI.h>
#include <U8g2lib.h>
#include <SoftwareSerial.h> // soft serial

#include <Bounce2.h>

#define rxButton A2 // soft serial
#define txButton A3 // soft serial
#define LiftUpButton 12        //Sıcaklık Artırma Butonu
#define LiftDownButton 9      //Sıcaklık Azaltma Butonu
#define StartButton 2
#define StopButton 3
#define TimeUpButton 4
#define TimeDownButton 5
#define DistanceUpButton 6
#define DistanceDownButton 7
#define LiftDownRelay A0        //Alçaltma rölesi
#define LiftUpRelay A1          //Yükseltme rölesi

#define NumOfButtons 8

U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 8);

//const uint8_t BUTTON_PINS[NumOfButtons] = {4, 5, 6, 7, 2, 3};
const uint8_t BUTTON_PINS[NumOfButtons] = {TimeUpButton, TimeDownButton, DistanceUpButton, DistanceDownButton, StartButton, StopButton, LiftUpButton, LiftDownButton};

Bounce * buttons = new Bounce[NumOfButtons];

boolean Start;          //0 Stop, 1 Start
int Distance;       // mm cinsinden
int LCDDistance;    // cm cinsinden
int sifirPozisyonu;
int TimeInterval;   //Working time saniye
int  LCDTimeInterval;//Working time dakika
//int sicaklik;
int isi;
boolean newData = false;
boolean isLiftUp;
boolean isLiftDown;

int minute;
int second;
int bekleme;

boolean isLampReady = false;
boolean isWorking;
SoftwareSerial mySerial = SoftwareSerial (rxButton, txButton);
String LCDInfo;

//////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP /////
void setup() {
  Serial.begin(115200);
  Serial.println("Konsol: Start setup");

  for (int i = 0; i < NumOfButtons; i++) {
    //setup the bounce instance for the current buttons
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  );
    buttons[i].interval(5);              // interval in ms
  }

  pinMode(LiftUpButton, INPUT_PULLUP);
  pinMode(LiftDownButton, INPUT_PULLUP);

  pinMode(LiftUpRelay, OUTPUT);
  digitalWrite(LiftUpRelay, LOW); //Limit swiche kadar kalksın
  bekleme = millis();// 1 dk sonra kapanacak
  pinMode(LiftDownRelay, OUTPUT);
  digitalWrite(LiftDownRelay, HIGH);

  pinMode (rxButton, INPUT);
  pinMode (txButton, OUTPUT);
  mySerial.begin(9600);

  /////////////////////////////////////////   default values ////////////////////////////////
  isWorking = false;
  Start = 0; //0 Stop, 1 Start
  Distance = 600; // mm
  LCDDistance = 60; // cm LCD de gösterim
  sifirPozisyonu = 400; // mm
  LCDTimeInterval = 4; //Calisma suresi 4 dk default LCD de gösterim
  TimeInterval = LCDTimeInterval * 60; // Calisma suresi 240 sn default (4x60)
  //sicaklik = 80; //80 derece
  isi = 0;

  minute = 4; //Calisma suresi 4 dk default
  second = 0;
  bekleme = 0;

  isLiftUp = true;
  isLiftDown = false;
  /////////////////////////////////////////   default values ////////////////////////////////

  u8g2.begin();
  u8g2.setFontMode(0);
  LCDInfo = "Sistem Kontrol";
  printValues2LCD();

  // delay(1000);
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
    // 1 dk bekle. Bu arada limit sw çalışmış olur. Sonra Lift Up rölesini bırak
    if (!isWorking && bekleme > 0 && millis() >= bekleme +  60000) // 1 dk = 60 sn x 1000 ms
    {
      digitalWrite(LiftUpRelay, HIGH);
      isWorking = false;
      bekleme = 0;
    }
    // printValues2LCD();
  }
}

////////////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP /////
void loop()
{
  if (!isLampReady) {
    delay(2000);
    isLampReady = true;
  }
  commWithSerial();
  for (int i = 0; i < NumOfButtons; i++)  {
    // Update the Bounce instance :
    buttons[i].update();
    // Serial.println(buttons[i].read());
  }
  processButtons();

  //printValues2LCD();
}

// Bu fonksiyonun işlevlerini radio olmadan yapalım
// Lamba ile mesajlaşma işleri
void commWithSerial() {
  String message = "";
  int msgLength = 0;
  if (mySerial.available() > 0) {
    message = mySerial.readString();
    msgLength = message.length();
    Serial.print("Gelen mesaj: ");  Serial.print(message);Serial.print("  mesaj boyu : ");Serial.println(msgLength);
    if (message.startsWith("<LMD1>")) {
      LCDInfo = "Lift indiriliyor";
      isWorking = true;
      if (isLiftUp) {
        isLiftUp = false;
        digitalWrite(LiftUpRelay, HIGH);
      }
      // LMP mesajı 1 geldi. İndirme işemini başlat
      isLiftDown = true;
      digitalWrite(LiftDownRelay, LOW);

    }
    if (message.startsWith("<LMD0>")) {
      isWorking = true;
      LCDInfo = "Lift durduruldu.";
      isLiftUp = false;
      isLiftDown = false;
      digitalWrite(LiftUpRelay, HIGH);
      // LMP mesajı 0 geldi. İndirme işemini durdur.
      digitalWrite(LiftDownRelay, HIGH);
      isWorking = true;
    }
    if (message.startsWith("<LMU1>")) {
      isWorking = true;
      LCDInfo = "Lift kaldırılıyor";
      if (isLiftDown) {
        digitalWrite(LiftDownRelay, HIGH);
        isLiftDown = false;
        delay(100);
      }
      // LMU mesajı 1 geldi. Kaldırma işlemine başla
      isLiftUp = true;
      digitalWrite(LiftUpRelay, LOW);
      bekleme = millis();
    }
    if (message.startsWith("<LMU0>")) {
      isWorking = true;
      LCDInfo = "Lift durduruldu.";
      isLiftUp = false;
      isLiftDown = false;
      digitalWrite(LiftDownRelay, HIGH);
      // LMU mesajı 0 geldi. Kaldırma işlemini durdur
      digitalWrite(LiftUpRelay, HIGH);
      isWorking = false;
    }
    if (message.startsWith("<ISI")) {
      String Tmp = message.substring(5, message.length());
      isi = Tmp.toInt();
    }
    if (message.startsWith("<LRD1>")) {
      isLampReady = true;
      if (mySerial.available() > 0)
        message = mySerial.readString();
      Serial.println("Lift Hazır.");
    }
    Serial.println(LCDInfo);
    printValues2LCD();
  }
}

void processButtons() {
  String message;

  message = "";
  // Serial.print("Buton basıldı"); Serial.println(buton);
  if (!isWorking)
  {
    if (buttons[0].fell() ) //TimeUpButton
    {
      LCDTimeInterval++;
      minute = LCDTimeInterval;
      second = 0;
      TimeInterval = LCDTimeInterval * 60; //Saniyeye çevir
      message = "<TIM" + String(TimeInterval) + ">"; //TimeInterval saniye cinsinden iletilir.
      mySerial.print(message);
      delay(1000);
      //     Serial.print("TimeInterval : "); Serial.print(TimeInterval); Serial.print(" LCD Interval: "); Serial.println(LCDTimeInterval);
    }
    if  (buttons[1].fell() ) //TimeDownButton
    {

      LCDTimeInterval--;
      minute = LCDTimeInterval;
      second = 0;
      TimeInterval = LCDTimeInterval * 60; //Saniyeye çevir
      message = "<TIM" + String(TimeInterval) + ">"; //TimeInterval saniye cinsinden iletilir.
      mySerial.print(message);
      delay(1000);
      //     Serial.print("TimeInterval : "); Serial.print(TimeInterval); Serial.print(" LCD Interval: "); Serial.println(LCDTimeInterval);
    }
    if (buttons[2].fell()) //DistanceUpButton
    {
      LCDDistance++;
      Distance = LCDDistance ; //mm çevir
      message = "<DST" + String(Distance) + ">";     // Mesafe cm cinsinden iletilir
      mySerial.print(message);
      delay(1000);
      //    Serial.print("Distance : "); Serial.print(Distance); Serial.print(" LCD Distance : "); Serial.println(LCDDistance);
    }
    if (buttons[3].fell() ) //DistanceDownButton
    {
      if (LCDDistance > 0) {
        LCDDistance--;
      }
      Distance = LCDDistance ; //mm çevir
      message = "<DST" + String(Distance) + ">";     // Mesafe cm cinsinden iletilir
      mySerial.print(message);
      delay(1000);
      //    Serial.print("Distance : "); Serial.print(Distance); Serial.print(" LCD Distance : "); Serial.println(LCDDistance);
    }
    if (buttons[4].fell()) //StartButton
    {
      Serial.println("Start basildi...." );
      isWorking = true;
      minute = LCDTimeInterval;
      second = 0;

      /*
            // Lambaya mesaj yolla
            message = "<HEA" + String(sicaklik) + ">";
            mySerial.print(message);

      */
      // Lambaya mesaj yolla
      mySerial.print("<CMD1>");
      delay(1000);
      LCDInfo = "CALISIYOR";
    }
  }

  if (isWorking) //Eğer çalışıyorsa Stop pini kontrol edilir. Diğerlerine bakılmaz.
  {
    if (buttons[5].fell()) //StopButton
    {
      Serial.println("Stop basildi...." );
      digitalWrite(LiftDownRelay, HIGH);
      digitalWrite(LiftUpRelay, HIGH);

      mySerial.print("<CMD0>");
      delay(1000);
      minute = LCDTimeInterval;
      second = 0;
      isWorking = false;
      LCDInfo = "DURDURULDU";
    }
  }
  ///////////////////////////// LIFT UP TUSU //////////////////////////////////////////////
  if (buttons[6].fell() && digitalRead(LiftDownButton) != LOW ) //LiftUpButton press
  {
    if (isLiftDown) {
      isLiftDown = false;
      //      Serial.println("Down kapat");
      digitalWrite(LiftDownRelay, HIGH); // Yukarı rölesi çekili olabilir. Kapat
      delay(100);
    }
    isLiftUp = true;
    digitalWrite(LiftUpRelay, LOW);
  }
  if (buttons[6].rose()) //LiftUpButton reease
  {
    isLiftUp = false;
    digitalWrite(LiftUpRelay, HIGH);
  }

  ///////////////////////////// LIFT DOWN TUSU //////////////////////////////////////////////

  if (buttons[7].fell() && digitalRead(LiftUpButton) != LOW ) //LiftUpButton press
  {
    if (isLiftUp) {
      isLiftUp = false;
      //      Serial.println("Down kapat");
      digitalWrite(LiftUpRelay, HIGH); // Yukarı rölesi çekili olabilir. Kapat
      delay(100);
    }
    isLiftDown = true;
    digitalWrite(LiftDownRelay, LOW);
  }
  if (buttons[7].rose()) //LiftUpButton reease
  {
    isLiftDown = false;
    digitalWrite(LiftDownRelay, HIGH);
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
    /*
        u8g2.setCursor(0, 20);
        u8g2.print(F("SICAKLIK: "));
        u8g2.print(sicaklik);
    */
    u8g2.setCursor(0, 20);
    u8g2.print(F("SICAKLIK: "));
    u8g2.print(isi);

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

    //u8g2.setCursor(0, 40);
    u8g2.setCursor(80, 20);
    u8g2.print(LCDInfo);

    u8g2.setCursor(0, 55);
    u8g2.print(F("www.brsservis.com"));
  } while ( u8g2.nextPage() );
}
