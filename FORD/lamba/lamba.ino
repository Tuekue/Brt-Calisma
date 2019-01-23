/*

  Cihazlar arası haberleşme için kullanılacak komut seti
  'CMD 1' start process
  'CMD 0' stop process
  'LMU 1' lower lamp
  'LMU 0' stop lowering lamp
  'LMD 1' raise lamp
  'LMD 0 stop rising lamp
  'DST XXX' minDistance set to/read XXX (0-999 mm)
  'TIM XXX' Lamp Working time set/remaining
  'HEA XXX' Ayarlanan Sıcaklık
  'ISI XXX' Ölçülen Sıcaklık
  'LRD1 Lamba ready
  'CRD1 Konsol ready
*/

#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <SoftwareSerial.h> // soft serial
#include "NewPing.h"
#include <Adafruit_MLX90614.h> //Isı sensörü
Adafruit_MLX90614 mlx = Adafruit_MLX90614();


// Hook up HC-SR04 with Trig to Arduino Pin 10, Echo to Arduino pin 13
// Maximum Distance is 400 cm

#define TRIGGER_PIN  9
#define ECHO_PIN     10
#define MAX_DISTANCE 400

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

#define rxPin 4           // RX soft serial
#define txPin 3           // TX soft serial
//A4 (SDA), A5 (SCL)
#define LampRelay 12        //Lamba rölesi
#define FanRelay 13        //Fan rölesi

//#define CE 9
//#define CSN 10
//#define MOSI 11
//#define MISO 12
//#define SCK 13

#define DIST_SCL A4
#define DIST_SDA A5

boolean Start; //0 Stop, 1 Start
int minDistance; //in mm
int TimeInterval; //Working time

int timerSayac;
boolean startTimer; //Geri sayım başlasın mı
boolean isWorking; //Çalışır durumda mı
String message;
boolean newData = false;

int sicaklik;
unsigned long bekleme = 0;
boolean isConsoleReady = false;

boolean buttonState = 0;
SoftwareSerial mySerial = SoftwareSerial (rxPin, txPin);

//////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP /////
void setup() {
  Serial.begin(115200);

  Serial.println("Lamba: Start setup");
  pinMode(LampRelay, OUTPUT);
  pinMode(FanRelay, OUTPUT);
  digitalWrite(LampRelay, HIGH);
  digitalWrite(FanRelay, HIGH);

  mlx.begin();  //Isı sensoru gy-906

  pinMode (rxPin, INPUT);
  pinMode (txPin, OUTPUT);
  mySerial.begin(9600);

  //default values
  isWorking = false;
  startTimer = false;
  Start = 0; //0 Stop, 1 Start
  minDistance = 60; //in cm
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
}

/////////////////////////////////////////////////////////////////////////////
/* Arduino otomatik olarak her saniye aşağıdaki fonksiyonu çalıştıracaktır */
/* Time interrupt function                                                 */
/////////////////////////////////////////////////////////////////////////////
ISR(TIMER1_COMPA_vect) {
  /*sicaklik = mlx.readObjectTempC();
    String isi = "<ISI" + sicaklik;
    mySerial.print(isi);
  */
  if (isWorking && startTimer) {
    //Serial.print("timerSayac: "); Serial.println(timerSayac);
    if (timerSayac > 0) {
      timerSayac--;
    } else {
      startTimer = false;
    }
  }
  //  Serial.print("isWorking: "); Serial.print(isWorking);
  //  Serial.print("millis: "); Serial.print(millis());
  //  Serial.print("bekleme++: "); Serial.println(bekleme + 18000);
  //  Serial.print("bekleme: "); Serial.println(bekleme);

  // 3 dk bekle. Bu süre içerisinde aşağıdaki kupanın gitmesi gerekiyor.
  // Yoksa yeni kupa sanıp tekrar başlıyor.
  if (bekleme > 0 && millis() >= bekleme + 180000) // 3  x 60 sn x 1000 ms
  {
    bekleme = 0;
    isWorking = false;
  }
}

////////////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP /////
void loop() {
  if (!isConsoleReady) {
    delay(5000);
    isConsoleReady = true;
  }
  // Konsoldan gelen bir komut/bilgi var mı kontrol et
  commWithSerial();

  if (!isWorking && bekleme == 0) { //Sistem çalışır durumda değil ve bekleme sürecinde değilse
    if (kupaVarMi()) {
      isWorking = true;
      Serial.println("Kupa geldi... Çalış");
    }
  }
  // çalışma komutu ya da kupa geldi ise isWorking true olmalı, o zaman program başlasın.
  if (isWorking) {
    runProgram();
  }
  //Serial.print("*C\tObject = "); Serial.print(sicaklik); Serial.println("*C");
}
////////////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP /////

//lamba yukarıda beklerken, Aşağı bir kupa gelip gelmediğini kontrol ederken kullanılıyor.
bool kupaVarMi() {
  int sayac = 0; // 5 kez dıgru bir sekilde okumasını saglamak icin
  int test = 0;  // max kaç kere test edecek
  float mesafe;

  // 5 kere denemeden geçme
  while (sayac < 5 && test < 20)
  {
    mesafe = readDistance();
    if (mesafe >= minDistance && mesafe < 160) {//160 cm yukarıda dururken altına birsey geldi. 60cmden yüksek olduğundan emin ol
      sayac++;
    } else {
      sayac = 0;
    }
    // Serial.print("Sayac: "); Serial.println(sayac);
    Serial.print("Bekleme Mesafe: "); Serial.println(mesafe);
    test++;
  }

  // yeterince doğru mesafe okundu ise aşağıda bir şey var
  if (sayac > 4) {
    return true;
  } else {
    return false;
  }
}


int readDistance() {
  float okunanMesafe;
  okunanMesafe = sonar.ping_cm();
  //Serial.print("Okunan mesafe: "); Serial.println(okunanMesafe);
  return okunanMesafe;
}

void commWithSerial() {
  String msg;
  //  Serial.println("Seri okuma başlangıç");
  if (mySerial.available() > 0)
  {
    message = mySerial.readString();
    Serial.print("Gelen Mesaj = "); Serial.println(message);
    if (message.startsWith("<DST")) {
      msg = message.substring(5, 8);
      minDistance = msg.toInt();
      Serial.print("minDistance = "); Serial.println(minDistance);
    }
    if (message.startsWith("<HEA")) {
      msg = message.substring(5, 8);
      sicaklik = msg.toInt();
      Serial.print("sicaklik = "); Serial.println(sicaklik);
    }
    if (message.startsWith("<TIM")) {
      msg = message.substring(5, 8);
      TimeInterval = msg.toInt();
      Serial.print("TimeInterval = "); Serial.println(TimeInterval);
    }

    if (message.startsWith("<CMD1>")) {
      isWorking = true;
    }
    if (message.startsWith("<CMD0>")) {
      stopAndReset();
    }
    if (message.startsWith("<CRD1>")) {
      isConsoleReady = true;
      Serial.println("Konsol Hazır: ");
    }
  }
}

//wayToTruck, lamba aşağı inerken kullanılıyor. Kupaya olan mesafe limit değer olana kadar aşağı in
int wayToTruck() {
  int  sayac = 0;
  int test;
  float mesafe;


  while (sayac < 5 )
  {
    mesafe = readDistance();
    if (mesafe <= minDistance ) {//160 cm yukarıda dururken altına birsey geldi. 60cmden yüksek olduğundan emin ol
      sayac++;
    } else {
      sayac = 0;
    }
    //    Serial.print("minDistance: "); Serial.println(minDistance);
    //   Serial.print("Kupaya kalan Mesafe: "); Serial.println(mesafe);

    commWithSerial();
  }
  if (sayac > 4) {
    return true;
  } else {
    return false;
  }
}
//Karşıdan stop tuşu yada reset algılandığında herşeyi güvenli hale getir.
void stopAndReset() {
  isWorking = false;
  startTimer = false;
  timerSayac = TimeInterval;
  digitalWrite(LampRelay, HIGH);
  digitalWrite(FanRelay, HIGH);

  // Limit switch gelince duracak. Biz elleşmiyoz.

}

void runProgram() {
  //Serial.print("startTimer"); Serial.println(startTimer);
  if (!startTimer && bekleme == 0) {

    // 1.adım İndirme işemini başlat
    //Serial.println("1.adım İndirme işemini başlat");
    // Lamba aşağı mesajını konsola yolla ki motoru çalıştırsın
    Serial.println("Lamba aşağı");
    mySerial.print("<LMD1>");
    delay(500);

    // Okunan Mesafe minDistance'dan küçük olduğu sürece bekle. Default 600mm
    wayToTruck();

    // Lamba aşşağı bitti mesajını konsola yolla ki motoru durdursun
    Serial.println("Lift motoru durdur");
    mySerial.print("<LMD0>");
    delay(500);

    // 2.adım Lambayı Yak
    Serial.println("2.adım Lambayı Yak ");
    digitalWrite(LampRelay, LOW);
    digitalWrite(FanRelay, LOW);
    // 3.adım Süreyi başlat
    timerSayac = TimeInterval;
    Serial.println("3.adım Süreyi başlat ");
    startTimer = true;
  }
  String mesaj;
  if (startTimer && bekleme == 0) {
    /*
      Serial.print("Timersayac:"); Serial.print(timerSayac);
      Serial.print(" bekleme:"); Serial.print(bekleme);
      Serial.print( "startTimer:"); Serial.println(startTimer);
    */
    if (timerSayac == 0) {
      // 4.adım Süre bitti ise Lambayı kapat, Yukarı kaldır
      stopAndReset() ;

      //Lamba yukarı mesajını yolla ki motor çalıştırsın
      Serial.println("Lift Motor yukarı");
      mySerial.print("<LMU1>");
      delay(500);
      // Limit switch gelince duracak. Biz elleşmiyoz.

      bekleme = millis();
    }
  }
}

