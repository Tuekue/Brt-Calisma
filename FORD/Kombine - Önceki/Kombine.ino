#include "Arduino.h"
#include <SPI.h>
#include <U8g2lib.h>

#include <Wire.h>
#include <I2C_graphical_LCD_display.h>

#include <Bounce2.h>
#include <VL53L0X.h>
#include "NewPing.h"
#include <Adafruit_MLX90614.h> //Isı sensörü
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Hook up HC-SR04 with Trig to Arduino Pin 10, Echo to Arduino pin 13
// Maximum Distance is 400 cm

#define TRIGGER_PIN  14
#define ECHO_PIN     15
#define MAX_DISTANCE 400

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

#define NumOfButtons 8
////////***************////////////////
#define StartButton 3
#define StopButton 4
#define TimeUpButton 5
#define TimeDownButton 6
#define DistanceUpButton 7
#define DistanceDownButton 8
#define HeatUpButton 9         // Sicaklik Yükselt butonu
#define HeatDownButton 10       // Sicaklik Düşür butonu

////// Number of Relay Pins
#define LampRelay 11            //Lamba rölesi
//#define FanRelay 12             //Fan rölesi
#define LiftDownRelay 16        //Alçaltma rölesi
#define LiftUpRelay 17          //Yükseltme rölesi
//20 (SDA), 21 (SCL)

#define HEAT_SCL 21
#define HEAT_SDA 20

//clock=SCK data=MOSI CS=MISO
U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, /* clock=*/ 52, /* data=*/ 51, /* CS=*/ 50, /* reset=*/ 53);
// --BRS
//I2C_graphical_LCD_display lcd;

//const uint8_t BUTTON_PINS[NumOfButtons] = {4, 5, 6, 7, 2, 3, A4, A5};
const uint8_t BUTTON_PINS[NumOfButtons] = {TimeUpButton, TimeDownButton, DistanceUpButton, DistanceDownButton, StartButton,
                                           StopButton, HeatUpButton, HeatDownButton
                                          };

Bounce * buttons = new Bounce[NumOfButtons];

boolean Start;          //0 Stop, 1 Start
int minDistance;        // cm cinsinden
int LCDOperationTime;    //Working time dakika
int sicaklik;
int isi;
boolean newData = false;
boolean isLiftUp;
boolean isLiftDown;

int countDown;
int minute;
int second;

int heatSet;
int heatRead;
unsigned long bekleme = 0;

//int TimeInterval; //Working time

//int timerSayac;
boolean isTimerRun; //Geri sayım başlasın mı
boolean isTimerEnd; //Geri sayım bitti mi. Bunu sayım başlamış ama bitince yapılacak işleri kontrol için koyuyoruz
boolean isWorking; //Çalışır durumda mı

// --BRS
String LCDInfo;
//char * LCDInfo;


//////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP /////
void setup() {
  Serial.begin(115200);
  Serial.println("Kombine: Setup baslangici");

  mlx.begin();
  for (int i = 0; i < NumOfButtons; i++) {
    //setup the bounce instance for the current buttons
    buttons[i].attach(BUTTON_PINS[i] , INPUT_PULLUP  );
    buttons[i].interval(10);              // interval in ms
  }

  //============== Röle ayarları ==============
  pinMode(LampRelay, OUTPUT);
  //pinMode(FanRelay, OUTPUT);
  digitalWrite(LampRelay, HIGH);
  //digitalWrite(FanRelay, HIGH);

  pinMode(LiftUpRelay, OUTPUT);
  digitalWrite(LiftUpRelay, HIGH);
  //İlk açılışta liftimiz limit swiche kadar yada 2000ms kalksın
  pinMode(LiftDownRelay, OUTPUT);
  digitalWrite(LiftDownRelay, HIGH);

  //==============   default values   ==============
  isWorking = false;
  Start = 0; //0 Stop, 1 Start
  minDistance = 70; // cm LCD de gösterim
  LCDOperationTime = 4; //Calisma suresi 4 dk default LCD de gösterim

  sicaklik = 90; //90 derece default değer
  heatSet = sicaklik ; //90 derece default değer
  isi = 0;
  minDistance = 70; //Çalışmaya başlama mesafesi cm
  minute = LCDOperationTime; //Calisma suresi 4 dk default
  second = 0;
  countDown = minute * 60; //Saniye olarak sayaç

  isLiftUp = true;
  isLiftDown = false;
  isTimerRun = false;
  isTimerEnd = false;
  /////////////////////////////////////////   default values   ////////////////////////////////

  // --BRS
  //lcd.begin();
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

  Serial.println("Kombine: Setup bitti");
}

/////////////////////////////////////////////////////////////////////////////
/* Arduino otomatik olarak her saniye aşağıdaki fonksiyonu çalıştıracaktır */
/* Time interrupt function                                                 */
/////////////////////////////////////////////////////////////////////////////
ISR(TIMER1_COMPA_vect) {
  // Eğer çalışıyorsa süreyi azalt
  if (isTimerRun && countDown > 0) {
    isTimerEnd = false;
    if (countDown > 0) {
      countDown--;

      Serial.print("CountDown: "); Serial.println(countDown);
      isiKontrol();
      Serial.print(".");
      if (second > 0) {
        second--;
      } else {
        minute-- ;
        second = 59;
        //Eğer süre bitti ise işlemi durdur süreyi sıfırla.
        if (minute < 0 )
        {
          countDown = 0;
          isTimerEnd = true;
        }
      }
    }
  }
  //1 dk sonra liftup rölesini bırakacak. Liftin yukarı çıkması için
  if (bekleme > 0 && millis() >= bekleme + 60000) // 1 dk = 60 sn x 1000 ms
  {
    Serial.println("Stop&Reset");
    stopAndReset();
  }
  // printValues2LCD();

}

////////////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP /////
void loop()
{
  // Update the Bounce instance :
  for (int i = 0; i < NumOfButtons; i++)  {
    buttons[i].update();
  }
  processButtons();

  //TODO : Start butonu basılınca çalışsın
  if (Start == 1) {
    if (kupaVarMi()) {
      Serial.println("Kupa geldi... Çalış");
      isWorking = true;
    }
  }
  // çalışma komutu ya da kupa geldi ise isWorking true olmalı, o zaman program başlasın.
  if (isWorking) {
    runProgram();
  }

  heatRead = mlx.readObjectTempC();
  heatRead = mlx.readObjectTempC();
  //Serial.print("Isı : "); Serial.println(heatRead);
  printValues2LCD();
}

void runProgram() {
  /*
      Serial.print("isTimerRun"); Serial.println(isTimerRun);
      Serial.print("isWorking"); Serial.println(isWorking);
  */

  Start = 0;
  //Geri sayım başlamamış
  isTimerEnd = false;
  if (!isTimerRun) {
    // 1.adım İndirme işemini başlat
    Serial.println("1.adım İndirme işemini başlat");
    // Lamba aşağı mesajını konsola yolla ki motoru çalıştırsın
    Serial.println("Lamba aşağı");
    LCDInfo = "Lift indiriliyor";
    // Lift kaldırılıyorsa önce onu durdur.
    if (isLiftUp) {
      isLiftUp = false;
      digitalWrite(LiftUpRelay, HIGH);
    }
    // İndirme işemini başlat
    isLiftDown = true;
    digitalWrite(LiftDownRelay, LOW);
    delay(100);

    // Okunan Mesafe minDistance'dan küçük olduğu sürece bekle. Default 70 cm 60+10
    if (wayToTruck() != 0) { //sayac 0 ise stop geldi
      LCDInfo = "Lift durduruldu.";
      isLiftUp = false;
      isLiftDown = false;
      digitalWrite(LiftUpRelay, HIGH);
      digitalWrite(LiftDownRelay, HIGH);

      // 2.adım Lambayı Yak
      Serial.println("2.adım Lambayı Yak ");
      digitalWrite(LampRelay, LOW);
      //      digitalWrite(FanRelay, LOW);
      // 3.adım Süreyi başlat
      Serial.println("3.adım Süreyi başlat ");
      isTimerRun = true;
    }
  }

  if (countDown == 0 && bekleme == 0) {
    //    if () {
    // 4.adım Süre bitti ise Lambayı kapat, Yukarı kaldır
    // 2.adım Lambayı Yak
    Serial.println("4.adım Lambayı Kapa ");
    digitalWrite(LampRelay, HIGH);

    //    Serial.print("isTimerRun "); Serial.print(isTimerRun); Serial.print("\tisTimerEnd "); Serial.print(isTimerEnd); Serial.print("\tcountDown "); Serial.print(countDown);
    Serial.println("\nLift Motor yukarı");
    LCDInfo = "Lift kaldırılıyor";
    if (isLiftDown) {
      digitalWrite(LiftDownRelay, HIGH);
      isLiftDown = false;
      delay(100);
    }
    isLiftUp = true;
    digitalWrite(LiftUpRelay, LOW);
    bekleme = millis();
    //    }
  }
}

// Belirlenen sicaklik değerinin +/- 2 derece aralığında lamba açık, değilse lamba kapalı
void isiKontrol() {
  String isi = String(heatRead);
  //  Serial.print("sicaklik : "); Serial.print(sicaklik); Serial.print("\tHeatRead : "); Serial.println(heatRead);
  if (heatRead > sicaklik + 2) {
    digitalWrite(LampRelay, HIGH);
    //digitalWrite(FanRelay, HIGH);
  }
  if (heatRead < sicaklik - 2) {
    digitalWrite(LampRelay, LOW);
    //    digitalWrite(FanRelay, LOW);
  }
  // printValues2LCD();
}

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

    Serial.print("Sayac: "); Serial.print(sayac);
    Serial.print("\tTest: "); Serial.print(test);
    Serial.print("\tBekleme Mesafe: "); Serial.println(mesafe);

    test++;
  }

  // yeterince doğru mesafe okundu ise aşağıda bir şey var
  if (sayac > 4) {
    return true;
  } else {
    return false;
  }
}
// Sensörden mesafe oku
int readDistance() {
  float okunanMesafe;
  okunanMesafe = sonar.ping_cm();
  //Serial.print("Okunan mesafe: "); Serial.println(okunanMesafe);
  return okunanMesafe;
}

//wayToTruck, lamba aşağı inerken kullanılıyor. Kupaya olan mesafe limit değer olana kadar aşağı in
int wayToTruck() {
  int  sayac = 0;
  int test;
  float mesafe;

  while (sayac < 5 && isWorking)
  {
    mesafe = readDistance();
    if (mesafe > 5 && mesafe <= minDistance ) {//160 cm yukarıda dururken altına birsey geldi. 60cmden yüksek olduğundan emin ol
      sayac++;
    } else {
      sayac = 0;
    }
    /*
      Serial.print("minDistance: "); Serial.print(minDistance);
      Serial.print("\tSayac: "); Serial.print(sayac);
      Serial.print("\tKupaya kalan Mesafe: "); Serial.println(mesafe);
    */
  }
  //Serial.println("***********************************************************************************************");
  if (sayac > 4) {
    return mesafe;
  } else {
    return 0;
  }
}

void processButtons() {
  String message;

  message = "";
  // Serial.print("Buton basıldı"); Serial.println(buton);
  if (!isWorking)
  {
    //////////////////////// ZAMAN + BUTONU /////////////////////////////////
    if (buttons[0].fell() ) //TimeUpButton
    {
      LCDOperationTime++;
      minute = LCDOperationTime;
      second = 0;
      countDown = minute * 60;
    }
    //////////////////////// ZAMAN - BUTONU /////////////////////////////////
    if  (buttons[1].fell() ) //TimeDownButton
    {
      LCDOperationTime--;
      minute = LCDOperationTime;
      second = 0;
      countDown = minute * 60;
    }
    //////////////////////// MESAFE + BUTONU /////////////////////////////////
    if (buttons[2].fell()) //DistanceUpButton
    {
      minDistance = minDistance + 5 ;
    }
    //////////////////////// MESAFE - BUTONU /////////////////////////////////
    if (buttons[3].fell() ) //DistanceDownButton
    {
      if (minDistance > 0) {
        minDistance = minDistance - 5; // Son istek ile 5 cm oynama yapılıyor.
      }
    }
    //////////////////////// START BUTONU /////////////////////////////////
    if (buttons[4].fell()) //StartButton
    {
      Serial.println("Start basildi...." );
      if (Start == 0) {
        Start = 1;
        minute = LCDOperationTime;
        second = 0;
        countDown = minute * 60;
        LCDInfo = "CALISIYOR";
      }
    }
    //////////////////////// SICAKLIK + BUTONU /////////////////////////////////
    if (buttons[6].fell()) //SetHeatUpButton
    {
      Serial.println("Sicaklik + basildi...." );
      sicaklik = sicaklik + 5;
      //Serial.print("sicaklik : "); Serial.print(sicaklik);
    }
    //////////////////////// SICAKLIK - BUTONU /////////////////////////////////
    if (buttons[7].fell() ) //SetHeatDownButton
    {
      Serial.println("Sicaklik - basildi...." );
      sicaklik = sicaklik - 5;
      if (sicaklik < 0)
      {
        sicaklik = 0;
      }
      //    Serial.print("sicaklik : "); Serial.print(sicaklik);
    }
  }
  //////////////////////// STOP BUTONU /////////////////////////////////
  if (buttons[5].fell()) //StopButton
  {
    Serial.println("Stop basildi...." );
    stopAndReset();
  }
  //printValues2LCD();
}

void stopAndReset() {
  Start = 0;
  isWorking = false;
  isTimerRun = false;
  digitalWrite(LampRelay, HIGH);
  //digitalWrite(FanRelay, HIGH);
  isLiftDown = false;
  digitalWrite(LiftDownRelay, HIGH);
  isLiftUp = false;
  digitalWrite(LiftUpRelay, HIGH);
  bekleme = 0;
  minute = LCDOperationTime;
  second = 0;
  countDown = minute * 60;
  LCDInfo = "DURDURULDU";
}

// Tum fonksiyon
// --BRS
/*
  void printValues2LCD() {
  char cstr[16];

  lcd.gotoxy(0, 10);
  lcd.string("MESAFE");

  lcd.gotoxy(65, 10);
  lcd.string(": ");
  itoa(minDistance, cstr, 10);
  lcd.string(cstr);

  lcd.gotoxy(0, 20);
  lcd.string("HEDEF ISI");
  lcd.gotoxy(65, 20);
  lcd.string(": ");
  itoa(sicaklik, cstr, 10);
  lcd.string(cstr);

  lcd.gotoxy(0, 30);
  lcd.string("YUZEY ISI");
  lcd.gotoxy(65, 30);
  lcd.string(": ");
  itoa(isi, cstr, 10);
  lcd.string(cstr);

  lcd.gotoxy(0, 40);
  //    lcd.gotoxy(0, 30);
  lcd.string("ZAMAN");
  lcd.gotoxy(65, 40);
  lcd.string(": ");
  if (minute < 10) {
    lcd.string("0");
  }

  itoa(minute, cstr, 10);
  lcd.string(cstr);
  lcd.string(":");

  if (second < 10) {
    lcd.string("0");
  }
  itoa(second, cstr, 10);

  lcd.string(cstr);

  lcd.gotoxy(0, 50);
  //lcd.gotoxy(0, 40);2
  lcd.string(LCDInfo);

  //    lcd.gotoxy(0, 55);
  lcd.gotoxy(0, 60);
  lcd.string("www.brsservis.com", true);
  }
*/
void printValues2LCD() {

  u8g2.setFont(u8g2_font_mozart_nbp_tr);
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 10);
    u8g2.print(F("MESAFE"));

    u8g2.setCursor(65, 10);
    u8g2.print(F(": "));
    u8g2.print(minDistance - 10);

    u8g2.setCursor(0, 20);
    u8g2.print(F("HEDEF ISI"));
    u8g2.setCursor(65, 20);
    u8g2.print(F(": "));
    u8g2.print(sicaklik);

    u8g2.setCursor(0, 30);
    u8g2.print(F("YUZEY ISI"));
    u8g2.setCursor(65, 30);
    u8g2.print(F(": "));
    u8g2.print(heatRead);

    u8g2.setCursor(0, 40);
    //    u8g2.setCursor(0, 30);
    u8g2.print(F("ZAMAN"));
    u8g2.setCursor(65, 40);
    u8g2.print(F(": "));
    if (minute < 10) {
      u8g2.print(F("0"));
    }
    u8g2.print(minute);
    u8g2.print(F(":"));

    if (second < 10) {
      u8g2.print(F("0"));
    }
    u8g2.print(second);

    u8g2.setCursor(0, 50);
    //u8g2.setCursor(0, 40);
    u8g2.print(LCDInfo);

    //    u8g2.setCursor(0, 55);
    u8g2.setCursor(0, 60);
    u8g2.print(F("www.brsservis.com"));
  } while ( u8g2.nextPage() );
}

