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
#include <SoftwareSerial.h> // soft serial
#define rxPin 4             // soft serial
#define txPin 3             // soft serial

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
int TimeInterval; //Working time

int timerSayac;
boolean startTimer;
boolean isWorking;
String message;
boolean newData = false;

int sicaklik;
int say = 0;
int bekleme = 0;

boolean buttonState = 0;
SoftwareSerial mySerial = SoftwareSerial (rxPin, txPin);

//////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP /////
void setup() {
  Serial.begin(9600);

  Serial.println("Start lamba");
  pinMode(RLamba, OUTPUT);
  pinMode(RFan, OUTPUT);
  digitalWrite(RLamba, HIGH);
  digitalWrite(RFan, HIGH);
  pinMode(13, OUTPUT);

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
  pinMode (rxPin, INPUT);
  pinMode (txPin, OUTPUT);
  mySerial.begin(9600);

  //default values
  isWorking = false;
  startTimer = false;
  Start = 0; //0 Stop, 1 Start
  LampPosition = 0; //1 down, 0 up
  Distance = 600; //in mm
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

/* Arduino otomatik olarak her saniye aşağıdaki fonksiyonu çalıştıracaktır */
ISR(TIMER1_COMPA_vect) {
  if (isWorking && startTimer) {
    //Serial.print("timerSayac: "); Serial.println(timerSayac);
    if (timerSayac > 0) {
      timerSayac--;
    }
    //süre bitti çalışma dursun
    //bu kısmı runProgram() fonksiyonu hallediyor

    if (bekleme > 0 && millis() >= bekleme +  180000) // 3dk = 3 x 60 sn x 1000 ms
    {
      digitalWrite(RLiftUp, HIGH);
      isWorking = false;
      bekleme = 0;
    }
  }
}

////////////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP /////
void loop() {

  // Konsoldan gelen bir komut/bilgi var mı kontrol et
  commWithSerial();

  // Çalışmıyorsan altta kupa var mıkontrol et. Mesafe 2000 mm (2 m) altında ise aşağıda bir şey var
  // sistem çalışmaya başlasın. (deneme yanılma ile düzeltilecek)

  if (!isWorking && mesafeOku(2000) < 2000 && say < 5) {
    isWorking = true;
    //Serial.println("Kupa geldi... Çalış");
  }

  // çalışma komutu ya da kupa geldi ise isWorking true olmalı, o zaman program başlasın.
  if (isWorking) {
    runProgram();
  }
}

int mesafeOku(int testValue) {
  if (bekleme > 0 && millis() >= bekleme +  180000) // 3dk = 3 x 60 sn x 1000 ms
  {
    bekleme = 0;
    mesafe=2000;
  } else {  
    int mesafe = readDistance();
    // 5 kere denemeden geçme
    mesafe = readDistance();
    if (mesafe < testValue) {
      say++;
    } else {
      say = 0;
    }
  }
  Serial.print("Mesafe: "); Serial.println(mesafe);
  return mesafe;
}

void commWithSerial() {

  char receivedChars[9];
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;
  String msg;

  if (mySerial.available() > 0)
  {
    message = mySerial.readString();

    Serial.print("Msg = "); Serial.println(message);
    if (message.startsWith("<DST")) {
      msg = message.substring(4, 3);
      Distance = msg.toInt();
    }
    if (message.startsWith("<HEA")) {
      msg = message.substring(4, 3);
      sicaklik = msg.toInt();
    }
    if (message.startsWith("<TIM")) {
      msg = message.substring(4, 3);
      TimeInterval = msg.toInt();
    }
    if (message == "<CMD1>") {
      isWorking = true;
    }
    if (message == "<CMD0>") {
      isWorking = false;
    }
    blinkLed();
  }
}
void blinkLed() {
  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13, LOW);
  delay(50);
  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13, LOW);
  delay(50);
  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13, LOW);
  delay(50);
  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13, LOW);
  delay(50);
  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13, LOW);
  delay(50);
}

void runProgram() {
  char str[6];
  //Serial.print("startTimer"); Serial.println(startTimer);
  if (!startTimer) {

    // 1.adım İndirme işemini başlat
    //Serial.println("1.adım İndirme işemini başlat");
    // Lamba aşşağı mesajını konsola yolla ki motoru çalıştırsın
    mySerial.print ("<LMD1>");
    Serial.println("<LMD1>");
    delay(1000);
    blinkLed();
    // Okunan Mesafe Distance'dan küçük olduğu sürece bekle. Default 600mm
    //Serial.print("Distance: "); Serial.print(readDistance()); Serial.print("  test value : "); Serial.println(Distance);
    int mesafe = readDistance();
    int say = 0; // 5 kere denemeden geçme

    while (mesafe > Distance && say < 5) {
      mesafe = mesafeOku(Distance);
    }

    // Lamba aşşağı bitti mesajını konsola yolla ki motoru durdursun
    mySerial.print ("<LMD0>");
    Serial.println("<LMD0>");
    delay(1000);

    // 2.adım Lambayı Yak
    //Serial.println("2.adım Lambayı Yak ");
    digitalWrite(RLamba, LOW);
    digitalWrite(RFan, LOW);
    // 3.adım Süreyi başlat
    timerSayac = TimeInterval;
    Serial.println("3.adım Süreyi başlat ");
    startTimer = true;
  }

  if (startTimer) {
    //Serial.print("Timersayac:"); Serial.println(timerSayac);

    if (timerSayac == 0) {
      // 4.adım Süre bitti ise Lambayı kapat, Yukarı kaldır
      digitalWrite(RLamba, HIGH);
      digitalWrite(RFan, HIGH);
      //Lamba yukarı mesajını yolla ki motor çalıştırsın
      mySerial.print ("<LMU1>");
      Serial.println("<LMU1>");
      delay(1000);
      // Limit switch gelince duracak. Biz elleşmiyoz.
      startTimer = false;
      bekleme = millis();
    }
  }
}

void processMessage(String input) {
  input.toUpperCase();
  Serial.print("Gelen mesaj: "); Serial.println(input);
  if (!isWorking) {//Lamba meşgul değilse
    if (input == "<CMD1>")
    {
      // Start komutu geldi.
      isWorking = true;
    }
    if (input.startsWith("<LMP"))
    {
      //Serial.println("LMP gelmiş");
      // Lamba indir kaldır komutla olmaz
    }
    if (input.startsWith("<DST"))
    {
      //Serial.println("DST Gelmiş");
      String Tmp = input.substring(4, 3);
      Distance = Tmp.toInt();
    }
    if (input.startsWith("<TIM")) {
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
  //Serial.print("Mesafe : "); Serial.println(okunanMesafe);
  return okunanMesafe;
}
