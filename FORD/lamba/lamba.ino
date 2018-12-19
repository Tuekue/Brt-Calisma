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
#define _fanTime 60

RF24 radio(CE, CSN);

const byte addresses[][6] = {"00001", "00002"}; //RF modül adresleri

uint8_t RadioId;
boolean Start; //0 Stop, 1 Start
uint8_t LampPosition; //1 down, 0  up
uint8_t Distance; //in mm
uint8_t sifirPozisyonu;
uint8_t TimeInterval; //Working time
uint8_t fanTimer;

uint8_t timerSayac;
boolean startTimer;

struct RadioPacket
{
  boolean Start; //0 Stop, 1 Start
  uint8_t LampPosition; //1 down, 0 up
  uint8_t Distance; //in mm
  uint8_t TimeInterval; //Working time
};

RadioPacket _radioData;

char message[31]; // Can hold a 30 character string + the null terminator.
boolean isWorking;

boolean buttonState = 0;


void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  pinMode(RLamba, OUTPUT);
  pinMode(RFan, OUTPUT);

  //default values
  isWorking = false;
  startTimer = false;
  Start = 0; //0 Stop, 1 Start
  LampPosition = 0; //1 down, 0 up
  Distance = 600; //in mm
  sifirPozisyonu = 400; // in mm
  TimeInterval = 240; //Working time 4min default
  timerSayac = TimeInterval;
  fanTimer = _fanTime;

  radio.begin();
  radio.enableDynamicPayloads();
  radio.enableAckPayload();                         // We will be using the Ack Payload feature, so please enable it
  radio.enableDynamicPayloads();                    // Ack payloads are dynamic payloads

  radio.openWritingPipe(addresses[0]); // 00002
  radio.openReadingPipe(1, addresses[1]); // 00001
  radio.setPALevel(RF24_PA_MIN);
  radio.maskIRQ(1, 1, 0); //Mask all interrupts except the receive interrupt tx ok, tx fail, rx ready

  attachInterrupt(0, radioInterrupt, CHANGE);

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
    Serial.print("timerSayac: "); Serial.println(timerSayac);
    if (timerSayac > 0) {
      timerSayac--;
    }
    //süre bitti çalışma dursun
    //bu kısmı runProgram() fonksiyonu hallediyor
  }
}


void loop() {

  /*
    //çalışma anı ile ilgili bölüm
    radio.startListening();
    if (radio.available()) {
      while (radio.available()) {
        radio.read(&message, sizeof(message));

        Serial.print("Gelen Mesaj : "); Serial.println(message);
      }

      Serial.println("test");
      radio.stopListening();
      //message[31] = '\0';
      Serial.print("Gelen Mesaj : "); Serial.println(message);
      //gelen mesaja göre değerleri set et yada çalışmaya başla
      processMessage(message);
    }
  */

  // Altta kupa var mı. Mesafe 1500 mm (1.5m) altında ise aşağıda bir şey var
  // sistem çalışmaya başlasın. (50 mm gecikme payı. deneme yanılma ile düzeltilecek)
  //Serial.println(readDistance());
  if (readDistance() <= 1550) {
    isWorking = true;
    Serial.println("Kupa geldi... Çalış");
  }

  // çalışma komutu ya da kupa geldi ise isWorking true olmalı, o zaman program başlasın.
  if (isWorking) {
    Serial.println("çalışmaya başladık.");
    runProgram();
  }

}

void radioInterrupt()
{
  boolean txOk, txFail, rxReady;
  radio.whatHappened(txOk, txFail, rxReady);

  // Serial.print("Aha radyoda program başladı.");Serial.print(txOk);Serial.print( txFail);Serial.print( rxReady);
  if (rxReady)
  {
    //   Serial.println("Bakalım neymiş");
    radio.read(&message, sizeof(message));
    // message[31] = '\0';
    Serial.print("Gelen Mesaj : "); Serial.println(message);
    //gelen mesaja göre değerleri set et yada çalışmaya başla
    processMessage(message);

    /*
      // Eski örneğin kalan kısmı. Belki lazım olur
          if (packetSize == sizeof(_radioData))
          {
            _radio.readData(&_radioData);

            String msg = "Radio ";
            msg += _radioData.Start;
            msg += ", ";
            msg += _radioData.LampPosition;
            msg += " Failed TX, ";
            msg += _radioData.Distance;
            msg += " Brightness, ";
            msg += _radioData.TimeInterval;

            Serial.println(msg);
          }
          else if (packetSize == sizeof(_messageData))
          {
            _radio.readData(&_messageData);

            String msg = "Radio ";
            msg += _messageData.FromRadioId;
            msg += ", ";
            msg += String((char*)_messageData.message);

            Serial.println(msg);
          }*/
  }
}


void runProgram() {
  char msg[31];
  Serial.print("startTimer"); Serial.println(startTimer);
  if (!startTimer) {

    // 1.adım İndirme işemini başlat
    strncpy( msg, "LMD0", sizeof(msg) );
    msg[sizeof(msg) - 1] = 0;
    Serial.println("4444");
    radio.stopListening();
    Serial.println("I am writing");
    if (!radio.write(msg, sizeof(msg))) {
      Serial.println("Write Failed");
    }
    //radio.startWrite(msg, sizeof(msg), 0);
    Serial.println("5555");
    radio.startListening();
    Serial.println("1.adım İndirme işemini başlat");
    while (readDistance() < Distance) {
      Serial.print("Distance: "); Serial.print(readDistance()); Serial.print("  test value : "); Serial.println(Distance);
    }
    // Okunan Mesafe Distance'dan küçük olduğu sürece bekle. Default 600mm
    //   radio.stopListening();
    //Lamba inişini durdur.
    strncpy( msg, "LMD0", sizeof(msg) );
    msg[sizeof(msg) - 1] = 0;

    Serial.println("I am writing 2");
    radio.write(msg, sizeof(msg));
    //radio.startWrite(msg, sizeof(msg), 0);
    radio.startListening();
    Serial.println("Lamba inişini durdur. ");
    // 2.adım Lambayı Yak
    digitalWrite(RLamba, HIGH);
    digitalWrite(RFan, HIGH);
    Serial.println("2.adım Lambayı Yak ");
    // 3.adım Süreyi başlat
    timerSayac = TimeInterval;
    startTimer = true;
    Serial.println("3.adım Süreyi başlat ");
  }

  if (startTimer) {
    Serial.print("Timersayac:"); Serial.println(timerSayac);

    if (timerSayac == 0) {
      // 4.adım Süre bitti ise Lambayı kapat, Yukarı kaldır
      digitalWrite(RLamba, LOW);
      // İkinci Süreyi başlat 60sn

      radio.stopListening();
      strncpy( msg, "LMU1", sizeof(msg) );
      msg[sizeof(msg) - 1] = 0;
      //radio.startWrite(msg, sizeof(msg), 0);

      Serial.println("I am writing 3");
      radio.write(msg, sizeof(msg));
      radio.startListening();


      while (readDistance() > sifirPozisyonu);
      // sifirPozisyonu na gelene kadar kaldır.
      // Kaldırma rölesini kapat

      radio.stopListening();
      strncpy( msg, "LMU0", sizeof(msg) );
      msg[sizeof(msg) - 1] = 0;
      //radio.startWrite(msg, sizeof(msg), 0);

      Serial.println("I am writing 4");
      radio.write(msg, sizeof(msg));
      radio.startListening();

    }
    //    if (fanTimer == 0) {
    //      // İkinci süre bittiyse Fan rölesini kapat
    //      digitalWrite(RLamba, LOW);
    //      // Çalışmayı durdur ve süreleri sıfırla
    //      startTimer = false;
    //      isWorking = false;
    //      fanTimer = _fanTime;
    //      timerSayac = TimeInterval;
    //    }
  }
}

void processMessage(String input) {
  input.toUpperCase();

  if (!isWorking) {//Lamba meşgul değilse
    if (input.startsWith("CMD"))
    {
      Serial.println("CMD gelmiş");
      if (input.substring(4, 1) == "1") {
        // Start komutu geldi.
        isWorking = true;

      }
    }
    else if (input.startsWith("LMP"))
    {
      Serial.println("LMP gelmiş");
      // Lamba indir kaldır komutla olmaz
    }
    else if (input.startsWith("DST"))
    {
      Serial.println("DST Gelömiş");
      String Tmp = input.substring(4, 3);
      Distance = Tmp.toInt();
    }
    else if (input.startsWith("TIM")) {
      Serial.println("TIM Gelmiş");
      String Tmp = input.substring(4, 3);
      TimeInterval = Tmp.toInt();
      timerSayac = TimeInterval;
    }
  }
}

int readDistance() {
  int okunanMesafe;
  okunanMesafe = sensor.readRangeContinuousMillimeters();
  Serial.println("Mesafe : " + okunanMesafe);
  return sensor.readRangeContinuousMillimeters();
}
