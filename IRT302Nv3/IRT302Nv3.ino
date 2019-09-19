#include "Arduino.h"
#include <Bounce2.h>
#include <Wire.h>
#include <I2C_graphical_LCD_display.h>

const byte alamba32x32 [] PROGMEM = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0x80, 0x0, 0x0, 0xe0,
  0xe0, 0x0, 0x0, 0x80, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};
const byte blamba32x32 [] PROGMEM = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc8, 0xd8, 0xd1, 0x13, 0xe6, 0xf8, 0xd, 0x5, 0x86,
  0x6, 0x6, 0x86, 0x5, 0xd, 0xf8, 0xe6, 0x13, 0xd1, 0xd8, 0xc8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};
const byte clamba32x32 [] PROGMEM = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x6, 0x36, 0x12, 0x19, 0x7, 0xc, 0xf8, 0xc0,
  0xff, 0xfe, 0xc1, 0xf8, 0xc, 0x7, 0x19, 0x12, 0x36, 0x6, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};
const byte dlamba32x32 [] PROGMEM = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x3, 0x7,
  0x7, 0x3, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

#define NumOfButtons 6
////////***************////////////////
#define StartButton 2
#define StopButton 3
#define TimeUpButton 4
#define TimeDownButton 5
#define singleDoubleButton 6

////// Number of Relay Pins
#define LampRelay1 7            //Lamba rölesi
#define LampRelay2 8            //Lamba rölesi
//#define FanRelay1 9             //Fan rölesi
//#define FanRelay2 10             //Fan rölesi

I2C_graphical_LCD_display lcd;

//const uint8_t BUTTON_PINS[NumOfButtons] = {4, 5, 6, 7, 2, 3, A4, A5};
const uint8_t BUTTON_PINS[NumOfButtons] = {StartButton, StopButton, TimeUpButton, TimeDownButton, singleDoubleButton
                                          };

Bounce * buttons = new Bounce[NumOfButtons];

boolean Start;          //0 Stop, 1 Start
int LCDOperationTime;    //Working time dakika

int countDown;
int minute;
int second;
int BesSay = 5;

int lambSelection = 2; //Tek cift secimi. Default çift

char * LCDInfo;

boolean isTimerRun; //Geri sayım başlasın mı
boolean isTimerEnd; //Geri sayım bitti mi. Bunu sayım başlamış ama bitince yapılacak işleri kontrol için koyuyoruz
boolean isWorking ; //Çalışır durumda mı

//////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP ////////// SETUP /////
void setup() {
  Serial.begin(115200);
  Serial.println("IRT302N: Setup baslangici");

  for (int i = 0; i < NumOfButtons; i++) {
    //setup the bounce instance for the current buttons
    buttons[i].attach(BUTTON_PINS[i] , INPUT_PULLUP  );
    buttons[i].interval(5);              // interval in ms
  }

  /////////////////////////////////////////   Röle ayarları   ////////////////////////////////
  pinMode(LampRelay1, OUTPUT);
  //pinMode(FanRelay1, OUTPUT);
  digitalWrite(LampRelay1, HIGH);
  //digitalWrite(FanRelay1, HIGH);

  pinMode(LampRelay2, OUTPUT);
  //pinMode(FanRelay2, OUTPUT);
  digitalWrite(LampRelay2, HIGH);
  //digitalWrite(FanRelay2, HIGH);

  /////////////////////////////////////////   default values   ////////////////////////////////

  LCDOperationTime = 15; //Calisma suresi 6 dk default LCD de gösterim

  /////////////////////////////////////////   initial values   ////////////////////////////////
  isWorking = false;
  Start = 0; //0 Stop, 1 Start

  minute = LCDOperationTime; //Calisma suresi 15 dk default
  second = 0;
  countDown = minute * 60; //Saniye olarak sayaç

  isTimerRun = false;
  isTimerEnd = false;

  // --BRS
  ////////////////////////////// LCD ilklendirme ///////////////////////////////
  lcd.begin();
  lambaCizx2();
  LCDInfo = "CIFT LAMBA              ";
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

  Serial.println("Setup bitti");
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
  } else {
    //Calismiyorsa Info yazisini max 5 saniye sonra sil
    BesSay--;
    if (BesSay <= 0) {
      BesSay = 5;
      LCDInfo = "                        ";
    }
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

  if (Start == 1) {
    isWorking = true;
  }
  // çalışma komutu ya da kupa geldi ise isWorking true olmalı, o zaman program başlasın.
  if (isWorking) {
    runProgram();
  }

  printValues2LCD();
}

void lambaCizx2() {
  lcd.gotoxy (0, 0);
  lcd.blit (alamba32x32, sizeof alamba32x32);
  lcd.gotoxy (0, 8);
  lcd.blit (blamba32x32, sizeof blamba32x32);
  lcd.gotoxy (0, 16);
  lcd.blit (clamba32x32, sizeof clamba32x32);
  lcd.gotoxy (0, 24);
  lcd.blit (dlamba32x32, sizeof dlamba32x32);

  lcd.gotoxy (64, 0);
  lcd.blit (alamba32x32, sizeof alamba32x32);
  lcd.gotoxy (64, 8);
  lcd.blit (blamba32x32, sizeof blamba32x32);
  lcd.gotoxy (64, 16);
  lcd.blit (clamba32x32, sizeof clamba32x32);
  lcd.gotoxy (64, 24);
  lcd.blit (dlamba32x32, sizeof dlamba32x32);

}
void lambaCizx1() {
  lcd.gotoxy (0, 0);
  lcd.blit (alamba32x32, sizeof alamba32x32);
  lcd.gotoxy (0, 8);
  lcd.blit (blamba32x32, sizeof blamba32x32);
  lcd.gotoxy (0, 16);
  lcd.blit (clamba32x32, sizeof clamba32x32);
  lcd.gotoxy (0, 24);
  lcd.blit (dlamba32x32, sizeof dlamba32x32);

  lcd.clear (64, 0, 96, 28, 0x00);

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
    Serial.println("Lambayı Yak ");
    digitalWrite(LampRelay1, LOW);
    // digitalWrite(FanRelay1, LOW);

    //Eğer çift lamba seçili ise ikinci lambayı da yak
    if (lambSelection == 2)
    {
      digitalWrite(LampRelay2, LOW);
      // digitalWrite(FanRelay2, LOW);
    }
    // Süreyi başlat
    Serial.println("3.adım Süreyi başlat ");
    isTimerRun = true;
  }

  if (countDown == 0) {
    // 4.adım Süre bitti ise Lambaları kapat,
    Serial.println("Lambayı Kapa ");
    digitalWrite(LampRelay1, HIGH);
    digitalWrite(LampRelay2, HIGH);
    // digitalWrite(FanRelay1, LOW);
    // digitalWrite(FanRelay2, LOW);
  }
}

void processButtons() {
  String message;

  message = "";
  // Serial.print("Buton basıldı"); Serial.println(buton);
  if (!isWorking)
  {
    //////////////////////// ZAMAN + BUTONU /////////////////////////////////
    if (buttons[2].fell() ) //TimeUpButton
    {
      LCDOperationTime++;
      minute = LCDOperationTime;
      second = 0;
      countDown = minute * 60;
    }
    //////////////////////// ZAMAN - BUTONU /////////////////////////////////
    if  (buttons[3].fell() ) //TimeDownButton
    {
      LCDOperationTime--;
      minute = LCDOperationTime;
      second = 0;
      countDown = minute * 60;
    }

    //////////////////////// TEK/CIFT - BUTONU /////////////////////////////////
    if (buttons[4].fell() ) //DistanceDownButton
    {
      if (lambSelection == 2)
      {
        lambSelection = 1;
        // TODO: Bunlar nasil olacak
        lambaCizx1();
        LCDInfo = "TEK LAMBA               ";
      } else {
        lambSelection = 2;
        // TODO: Bunlar nasil olacak
        lambaCizx2();
        LCDInfo = "CIFT LAMBA              ";
      }
    }
    //////////////////////// START BUTONU /////////////////////////////////
    if (buttons[0].fell()) //StartButton
    {
      Serial.println("Start basildi...." );
      if (Start == 0) {
        Start = 1;
        minute = LCDOperationTime;
        second = 0;
        countDown = minute * 60;
        LCDInfo = "CALISIYOR               ";
      }
    }

  }
  //////////////////////// STOP BUTONU /////////////////////////////////
  if (buttons[1].fell()) //StopButton
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
  digitalWrite(LampRelay1, HIGH);
  //digitalWrite(FanRelay1, HIGH);
  digitalWrite(LampRelay2, HIGH);
  //digitalWrite(FanRelay2, HIGH);
  minute = LCDOperationTime;
  second = 0;
  countDown = minute * 60;
  LCDInfo = "DURDURULDU              ";
}

void printValues2LCD() {
  char cstr[16];

  lcd.gotoxy(0, 38);
  lcd.string("ZAMAN");
  lcd.gotoxy(45, 38);
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

  lcd.gotoxy(0, 48);
  lcd.string(LCDInfo);

  //    lcd.gotoxy(0, 55);
  lcd.gotoxy(0, 60);
  lcd.string("www.brsservis.com", true);
}
