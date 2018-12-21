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
#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;
#define RLamba A0         //Lamba rölesi
#define RFan A1           //Fan rölesi

#define CE 9
#define CSN 10
#define MOSI 11
#define MISO 12
#define SCK 13
#define DIST_SCL A4
#define DIST_SDA A5

boolean Start; //0 Stop, 1 Start
int LampPosition; //1 down, 0  up
int Distance; //in mm
int sifirPozisyonu;
int TimeInterval; //Working time

int timerSayac;
boolean startTimer;
boolean isWorking;
String message;

int sicaklik;

boolean buttonState = 0;

//////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP /////
void setup() {
  Serial.begin(9600);

  // Mini de Serial1 kullanılıyormuş.
  // Serial1.begin(9600);

  Serial.println("Start lamba");
  pinMode(RLamba, OUTPUT);
  pinMode(RFan, OUTPUT);

  Wire.begin();
  sensor.init();
  sensor.setTimeout(500);

  // Uzun mesafe yavaş hız okuma modu
  // ------------------------------
  sensor.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  // ------------------------------

  //default values
  isWorking = false;
  startTimer = false;
  Start = 0; //0 Stop, 1 Start
  LampPosition = 0; //1 down, 0 up
  Distance = 600; //in mm
  sifirPozisyonu = 400; // in mm
  TimeInterval = 240; //Working time 4min default
  timerSayac = TimeInterval;

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
  Serial.flush();
}

/* Arduino otomatik olarak her saniye aşağıdaki fonksiyonu çalıştıracaktır */
ISR(TIMER1_COMPA_vect) {
  if (isWorking && startTimer) {
    //Serial.print("timerSayac: "); Serial.println(timerSayac);
    if (timerSayac > 0) {
      timerSayac--;
    }
    //süre bitti çalışma dursun
    //bu kısmı runProgram() fonksiyonu hallediyor
  }
}

////////////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP /////
void loop() {

  // Konsoldan gelen bir komut/bilgi var mı kontrol et
  commWithSerial();

  // Çalışmıyorsan altta kupa var mıkontrol et. Mesafe 2000 mm (2 m) altında ise aşağıda bir şey var
  // sistem çalışmaya başlasın. (deneme yanılma ile düzeltilecek)
  if (!isWorking && readDistance() <= 2000) {
    isWorking = true;
    //Serial.println("Kupa geldi... Çalış");
  }

  // çalışma komutu ya da kupa geldi ise isWorking true olmalı, o zaman program başlasın.
  if (isWorking) {
    runProgram();
  }


}

void commWithSerial() {
  int i;
  char str[6];
  String msg;
  // Gelen bilgi var ise
  if (Serial.available())
  {
    i = 0;
    // mesajı oku
    delay(100); //allows all serial sent to be received together
    while (Serial.available() && i < 6) {
      //Mini de Serial1 kullanılıyormuş
      //str[i++] = Serial1.read();
      str[i++] = Serial.read();
    }
    str[i++] = '\0';
  }
  if (i > 0) {
    message = str;
    //Serial.println(message);
  }
  if (message.startsWith("DST")) {
    msg = message.substring(4, 3);
    Distance = msg.toInt();
  }

  if (message.startsWith("HEA")) {

    msg = message.substring(4, 3);
    sicaklik = msg.toInt();
  }
  if (message.startsWith("CMD1")) {
    isWorking = true;
  }
}

void runProgram() {
  char str[6];
  //Serial.print("startTimer"); Serial.println(startTimer);
  if (!startTimer) {

    // 1.adım İndirme işemini başlat
    //Serial.println("1.adım İndirme işemini başlat");
    // Lamba aşşağı mesajını konsola yolla ki motoru çalıştırsın


    //Mini de Serial1 kullanılıyormuş
    //Serial1.write("LMD1");
    Serial.write("LMD1");
    delay(50);

    // Okunan Mesafe Distance'dan küçük olduğu sürece bekle. Default 600mm
    while (readDistance() < Distance) {
      //Serial.print("Distance: "); Serial.print(readDistance()); Serial.print("  test value : "); Serial.println(Distance);
      ;
    }

    // Lamba aşşağı bitti mesajını konsola yolla ki motoru durdursun
    //Mini de Serial1 kullanılıyormuş
    //Serial1.write("LMD0");
    Serial.write("LMD0");
    delay(50);

    // 2.adım Lambayı Yak
    //Serial.println("2.adım Lambayı Yak ");
    digitalWrite(RLamba, HIGH);
    digitalWrite(RFan, HIGH);
    // 3.adım Süreyi başlat
    timerSayac = TimeInterval;
    //Serial.println("3.adım Süreyi başlat ");
    startTimer = true;
  }

  if (startTimer) {
    //Serial.print("Timersayac:"); Serial.println(timerSayac);

    if (timerSayac == 0) {
      // 4.adım Süre bitti ise Lambayı kapat, Yukarı kaldır
      digitalWrite(RLamba, LOW);
      //Lamba yukarı mesajını yolla ki motor çalıştırsın
      //Mini de Serial1 kullanılıyormuş
      //  Serial1.write("LMU1");
      Serial.write("LMU1");

      while (readDistance() > sifirPozisyonu);
      // sifirPozisyonu na gelene kadar kaldır.
      //Lamba yukarı bitti mesajını yolla ki motoru durdursun
      //Mini de Serial1 kullanılıyormuş
      //  Serial1.write("LMU0");
      Serial.write("LMU0");

    }
  }
}

void processMessage(String input) {
  input.toUpperCase();

  if (!isWorking) {//Lamba meşgul değilse
    if (input.startsWith("CMD"))
    {
      //Serial.println("CMD gelmiş");
      if (input.substring(4, 1) == "1") {
        // Start komutu geldi.
        isWorking = true;
      }
    }
    if (input.startsWith("LMP"))
    {
      //Serial.println("LMP gelmiş");
      // Lamba indir kaldır komutla olmaz
    }
    if (input.startsWith("DST"))
    {
      //Serial.println("DST Gelömiş");
      String Tmp = input.substring(4, 3);
      Distance = Tmp.toInt();
    }
    if (input.startsWith("TIM")) {
      //Serial.println("TIM Gelmiş");
      String Tmp = input.substring(4, 3);
      TimeInterval = Tmp.toInt();
      timerSayac = TimeInterval;
    }
  }
}

int readDistance() {

  long okunanMesafe;
  okunanMesafe = sensor.readRangeSingleMillimeters();

  // Mesafe sensöründe <40 ve >2000 değerler gözardı edilsin
  // Saçma rakamlar gelebiliyor 0 ya da 8160 gibi

  if (okunanMesafe > 2000) {
    okunanMesafe = 2000;
  }
  if (okunanMesafe < 40) {
    okunanMesafe = 40;
  }
  //  Serial.print("Mesafe : "); Serial.println(okunanMesafe);
  return okunanMesafe;
}
