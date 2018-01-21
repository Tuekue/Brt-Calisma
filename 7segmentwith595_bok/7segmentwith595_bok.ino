


const int digitPins[3] = {8, 12, 9};
int digitScan = 0;
int ones, tens, hundreds;


#define heatLATCH 10
#define heatCLK 13
#define heatDATA 11

#define heatDigitHundred 8
#define heatDigitTen 12
#define heatDigitOne 9


//This is the hex value of each number stored in an array by index num
//byte digitOne[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

byte digitOne[10] = {
  B00111111, //0
  B00000110, //1
  B01011011, //2
  B01001111, //3
  B01100110, //4
  B01101101, //5
  B01111101, //6
  B00000111, //7
  B01111111, //8
  B01101111  //9};

};
int digitBuffer[3] = {0};

void setup() {
  Serial.begin(9600);

  pinMode(heatLATCH, OUTPUT);
  pinMode(heatCLK, OUTPUT);
  pinMode(heatDATA, OUTPUT);

  Serial.println("setup complete");
}

void setHeater () {
  valup = digitalRead(plusPin);
  valdown = digitalRead(minusPin);

  if (valup == HIGH && prevvalup == LOW) //funcitons based off of button pulling input pin LOW
  {
    if ((!isRunning) && (millis() - lastBtnUp > transInt))
    {
      // increase heat
      targetHeat++;
      calcDigits(targetHeat);
      digitBuffer[2] = hundreds;
      digitBuffer[1] = tens;
      digitBuffer[0] = ones;
      displayHeatDigits();

      //sevseg.write(targetHeat);
      Serial.println(targetHeat);
    }
    lastBtnUp = millis();
  }
  prevvalup = valup;

  if (valdown == HIGH && prevvaldown == LOW) //funcitons based off of button pulling input pin LOW
  {
    if ((!isRunning) && (millis() - lastBtnDwn > transInt))
    {
      // decrease heat
      targetHeat--;
      calcDigits(targetHeat);
      digitBuffer[2] = hundreds;
      digitBuffer[1] = tens;
      digitBuffer[0] = ones;
      displayHeatDigits();

      //      sevseg.write(targetHeat);
      Serial.println(targetHeat);
    }
    lastBtnDwn = millis();
  }
  prevvaldown = valdown;

} //setHeater

void heaterFunction () {

  static int mlxReading = 0;
  if (isRunning) {
    mlxReading = round(mlx.readObjectTempC());

    calcDigits(mlxReading);
    digitBuffer[2] = hundreds;
    digitBuffer[1] = tens;
    digitBuffer[0] = ones;

    displayHeatDigits();
    //sevseg.write(mlxReading);

    if (mlxReading - targetDelta < targetHeat) {
      //Relay çekilsin heater açılsın

    }
    if (mlxReading + targetDelta > targetHeat ) {
      // Relay bırakılsın heater kapansın

    }

  } else {
    // targetHeat = 30;
    setHeater();
    calcDigits(30);
    digitBuffer[2] = hundreds;
    digitBuffer[1] = tens;
    digitBuffer[0] = ones;
    displayHeatDigits();

    //    sevseg.write(targetHeat);
  }
} //heaterFunction

void displayHeatDigits()
{

  for (byte j = 0; j < 3; j++)
    digitalWrite(digitPins[j], LOW);

  digitalWrite(heatLATCH, LOW);
  shiftOut(heatDATA, heatCLK, MSBFIRST, B11111111);
  digitalWrite(heatLATCH, HIGH);

  delayMicroseconds(100);
  digitalWrite(8, HIGH);

  digitalWrite(heatCLK, LOW);

  shiftOut(heatDATA, heatCLK, MSBFIRST, digitOne[digitBuffer[digitScan]]);

  digitalWrite(heatLATCH, HIGH);
  digitScan++;
  if (digitScan > 2) digitScan = 0;
  Serial.println(digitPins[digitScan]);

  //  digitalWrite(heatLATCH, LOW);
  //  digitalWrite(heatDigitOne, HIGH);
  //  shiftOut(heatDATA, heatCLK, MSBFIRST, ~digitOne[ones]); // digitRight
  //  digitalWrite(heatLATCH, HIGH);
  //  //delayMicroseconds(100);
  //
  //  digitalWrite(heatLATCH, LOW);
  //  digitalWrite(heatDigitTen, HIGH);
  //  shiftOut(heatDATA, heatCLK, MSBFIRST, ~digitOne[tens]); //
  //  digitalWrite(heatLATCH, HIGH);
  //  //delayMicroseconds(100);
  //
  //  digitalWrite(heatLATCH, LOW);
  //  digitalWrite(heatDigitHundred, HIGH);
  //  shiftOut(heatDATA, heatCLK, MSBFIRST, ~digitOne[hundreds]); // digitLeft
  //  digitalWrite(heatLATCH, HIGH);
}

void calcDigits(int num)
{
  ones = num % 10;
  if (num < 10) {
    tens = 0;
  }
  else {
    tens = num / 10;
    hundreds = num / 100;
  }
  return;
}

void countDown ()
{
  int num;
  calcDigits(minute);
  displayMinuteDigits(ones, tens);

  calcDigits(second);
  displaySecondDigits(ones, tens);
}

void displaySecondDigits(int one, int ten)
{
  digitalWrite(secondLATCH, LOW);
  shiftOut(secondDATA, secondCLK, MSBFIRST, ~digitOne[one]); // digitRight
  shiftOut(secondDATA, secondCLK, MSBFIRST, ~digitOne[ten]); // digitLeft
  digitalWrite(secondLATCH, HIGH);

}

void displayMinuteDigits(int one, int ten)
{
  digitalWrite(minuteLATCH, LOW);
  shiftOut(minuteDATA, minuteCLK, MSBFIRST, ~digitOne[one]); // digitRight
  shiftOut(minuteDATA, minuteCLK, MSBFIRST, ~digitOne[ten]); // digitLeft
  digitalWrite(minuteLATCH, HIGH);

}


void loop()
{

  digitalWrite(8, HIGH);

  digitalWrite(minuteLATCH, LOW);

  shiftOut(minuteDATA, minuteCLK, MSBFIRST, B01010101);

  digitalWrite(minuteLATCH, HIGH);

////  heaterFunction();

  delay (20);
}
