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

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0,  13, 11,  10, 8);

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
#define NumOfButtons 8

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

int minute;
int second;

boolean isWorking;
boolean startTimer;

//////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP /////
void setup() {
  Serial.begin(9600);
  Serial.println("Start konsol");

  for (int i = 0; i < NumOfButtons; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    butgate[i] = false;
  }
  pinMode(RLiftUp, OUTPUT);
  digitalWrite(RLiftUp, LOW);
  pinMode(RLiftDown, OUTPUT);
  digitalWrite(RLiftDown, LOW);

  //default values
  isWorking = false;
  startTimer = false;
  Start = 0; //0 Stop, 1 Start
  LampPosition = 0; //1 asagi, 0 yukari
  Distance = 600; // mm
  LCDDistance = 60; // cm LCD de gösterim
  sifirPozisyonu = 400; // mm
  LCDTimeInterval = 4; //Calisma suresi 4 dk default LCD de gösterim
  TimeInterval = 240; // Calisma suresi 240 sn default (4x60)
  sicaklik = 80; //80 derece

  minute = LCDTimeInterval;
  second = 0;

  u8g2.begin();
  u8g2.clearBuffer();          // clear the internal memory

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
    u8g2.setFont(u8g2_font_mozart_nbp_tr );
    u8g2.setCursor(0, 30);

    u8g2.print("TIME: ");
    if (minute < 10) {
      u8g2.print(0);
    }
    u8g2.print(minute);
    u8g2.print(":");
    if (second < 10) {
      u8g2.print(0);
    }
    u8g2.print(second);

    u8g2.setCursor(0, 40);
    u8g2.print("CALISIYOR");

    u8g2.sendBuffer();
    //printValues2LCD();
  }
}

////////////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP ////////// LOOP /////
void loop()
{
  //Serial.println(digitalRead(fifi));
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
  int i;
  char str[6];
  String message;

  // Gelen bilgi var ise
  if (Serial.available())
  {
    i = 0;
    // mesajı oku
    delay(100); //allows all serial sent to be received together
    while (Serial.available() && i < 6) {
      str[i++] = Serial.read();
    }
    str[i++] = '\0';
  }
  if (i > 0) {
    message = str;
  }

  if (message.startsWith("LMD")) {
    digitalWrite(RLiftUp, LOW);
    delay(3000);
    if (message.substring(4, 1) == "1") {
      // LMP mesajı 1 geldi. İndirme işemini başlat
      digitalWrite(RLiftDown, HIGH);
    }
    if (message.substring(4, 1) == "0") {
      // LMP mesajı 0 geldi. İndirme işemini durdur.
      digitalWrite(RLiftDown, LOW);
    }
  }
  if (message.startsWith("LMU")) {
    digitalWrite(RLiftDown, LOW);
    delay(3000);
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

void processButtons(int buton) {
  //  Serial.print("Buton basıldı"); Serial.println(buton);
  if (!isWorking)
  {
    if (buttons[buton] == TimeUpPin)
    {

      LCDTimeInterval++;
      minute = LCDTimeInterval;
      second = 0;
      TimeInterval = LCDTimeInterval * 60; //Saniyeye çevir
      lcdTimeDisplay();
      //Serial.print("TimeInterval : "); Serial.print(TimeInterval); Serial.print(" LCD Interval: "); Serial.println(LCDTimeInterval);
    }
    if  (buttons[buton] == TimeDownPin)
    {

      LCDTimeInterval--;
      minute = LCDTimeInterval;
      second = 0;
      TimeInterval = LCDTimeInterval * 60; //Saniyeye çevir
      lcdTimeDisplay();
      //Serial.print("TimeInterval : "); Serial.print(TimeInterval); Serial.print(" LCD Interval: "); Serial.println(LCDTimeInterval);
    }
    if (buttons[buton]  == DistanceUpPin)
    {
      LCDDistance++;
      Distance = LCDDistance * 10; //mm çevir
      lcdDistanceDisplay();
      //Serial.print("Distance : "); Serial.print(Distance); Serial.print(" LCD Distance : "); Serial.println(LCDDistance);
    }
    if (buttons[buton]  == DistanceDownPin)
    {
      if (LCDDistance > 0) {
        LCDDistance--;
      }
      Distance = LCDDistance * 10; //mm çevir
      lcdDistanceDisplay();
      //Serial.print("Distance : "); Serial.print(Distance); Serial.print(" LCD Distance : "); Serial.println(LCDDistance);
    }
    if (buttons[buton]  == heatDownPin)
    {
      sicaklik --;
      lcdSicaklikDisplay();
      //Serial.print("Sıcaklık : "); Serial.println(sicaklik);
    }
    if (buttons[buton]  == heatUpPin)
    {
      sicaklik ++;
      lcdSicaklikDisplay();
      //Serial.print("Sıcaklık : "); Serial.println(sicaklik);
    }
    String message;
    if (buttons[buton]  == StartPin)
    {
      Serial.println("Start Working....⌂" );
      isWorking = true;
      minute = LCDTimeInterval;
      second = 0;

      char str[6];

      // Lambaya mesaj yolla
      message = "TIM" + String(TimeInterval); //TimeInterval saniye cinsinden iletilir.
      delay(50);

      // Lambaya mesaj yolla
      message = "DST" + String(Distance);     // Mesafe cm cinsinden iletilir
      char* buf = (char*) malloc(sizeof(char) * message.length() + 1);
      message.toCharArray(buf, message.length() + 1);
      Serial.write(buf);
      //Freeing the memory
      free(buf);
      delay(50);

      // Lambaya mesaj yolla
      message = "HEA" + String(sicaklik);     // Mesafe cm cinsinden iletilir
      buf = (char*) malloc(sizeof(char) * message.length() + 1);
      message.toCharArray(buf, message.length() + 1);
      Serial.write(buf);
      //Freeing the memory
      free(buf);
      delay(50);

      // Lambaya mesaj yolla
      message = "CMD1"; // başla
      buf = (char*) malloc(sizeof(char) * message.length() + 1);
      message.toCharArray(buf, message.length() + 1);
      Serial.write(buf);
      //Freeing the memory
      free(buf);
    }
  }
  String message;
  if (isWorking)
  {
    if (buttons[buton]  == StopPin)
    {
      Serial.println("Stop Working...." );

      minute = LCDTimeInterval;
      second = 0;
      printValues2LCD();

      isWorking = false;
      // Lambaya mesaj yolla
      message = "CMD0"; // durdur
      char* buf = (char*) malloc(sizeof(char) * message.length() + 1);
      message.toCharArray(buf, message.length() + 1);
      Serial.write(buf);
      //Freeing the memory
      free(buf);

    }
  }
}
void lcdTimeDisplay() {
  //u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_mozart_nbp_tr ); // choose a suitable font
  u8g2.setCursor(0, 30);

  u8g2.print("TIME: ");
  if (minute < 10) {
    u8g2.print(0);
  }
  u8g2.print(minute);
  u8g2.print(":");
  if (second < 10) {
    u8g2.print(0);
  }
  u8g2.print(second);
  u8g2.setCursor(0, 55);
  u8g2.print(F("www.brsservis.com"));

  u8g2.sendBuffer();
  u8g2.setFontMode(0);
}
void lcdDistanceDisplay() {
  //u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_mozart_nbp_tr ); // choose a suitable font

  u8g2.setCursor(0, 10);
  u8g2.print("DIST: ");
  u8g2.print(LCDDistance);

  u8g2.sendBuffer();
  u8g2.setFontMode(0);
}
void lcdSicaklikDisplay() {
  //u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_mozart_nbp_tr ); // choose a suitable font

  u8g2.setCursor(0, 20);
  u8g2.print("TEMP: ");
  u8g2.print(sicaklik);
//  Serial.print("sicaklik:"); Serial.print(sicaklik); Serial.print(" ");

  u8g2.sendBuffer();
  u8g2.setFontMode(0);

}
void printValues2LCD() {

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_mozart_nbp_tr ); // choose a suitable font

  lcdDistanceDisplay();

  lcdSicaklikDisplay();

  lcdTimeDisplay();

  u8g2.setCursor(0, 55);
  u8g2.print(F("www.brsservis.com"));
  u8g2.sendBuffer();
  u8g2.setFontMode(0);
  u8g2.setFont(u8g2_font_cu12_hr);

}

void processMessage(String input) {
  input.toUpperCase();
  char buf[10];

  // Gelen mesajı işeyip LCD'de göster
}
