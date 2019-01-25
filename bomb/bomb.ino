#include <SPI.h>
#include <MFRC522.h>
#include <DigitLedDisplay.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

//Digit led display pins
#define DIN 11
#define LOAD 10
#define CLK 9

#define BUZZPIN A15
#define ALARMPIN 12
#define GRENADEPIN 13

//KEYPINS
// ROWS
#define KEY0 A0
#define KEY1 A1
#define KEY2 A2
#define KEY3 A3
// COLUMNS
#define KEY4 A4
#define KEY5 A5
#define KEY6 A6
#define KEY7 A7

//Pines de los cables
#define WIRE1 1
#define WIRE2 2
#define WIRE3 3
#define WIRE4 4

//Pines botones
#define RED_BTN 7
#define GREEN_BTN 8

bool bBUZZ = true;
bool bALARM = true;
bool bGRENADE = true;

bool DOMAIN[2] = {false, false};

bool bPASS = true;
String PASS = "123ABC";

bool bWIRE = false;
bool CUTTED_WIRE[4] = {false, false, false, false};
int tWIRE[4] = {2, 3, 3, 3};

bool bKEYS = false;

bool ENDGAME = false;
bool WIN = false;
unsigned long RELOJ = 30000;//5 min en centesimas

const byte rowsCount = 4;
const byte columsCount = 4;
char keys[rowsCount][columsCount] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[rowsCount] = { KEY0, KEY1, KEY2, KEY3 };
byte columnPins[columsCount] = { KEY4, KEY5, KEY6, KEY7 };

DigitLedDisplay ld = DigitLedDisplay(DIN, LOAD, CLK);
Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rowsCount, columsCount);
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  //Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  ld.setDigitLimit(8);//8 digitos

  pinMode(ALARMPIN, OUTPUT);
  pinMode(BUZZPIN, OUTPUT);
  pinMode(WIRE1, INPUT_PULLUP);
  pinMode(WIRE2, INPUT_PULLUP);
  pinMode(WIRE3, INPUT_PULLUP);
  pinMode(WIRE4, INPUT_PULLUP);
  pinMode(RED_BTN, INPUT_PULLUP);
  pinMode(GREEN_BTN, INPUT_PULLUP);
}

void setup2() {
  ld.setBright(15); //1-15
  digitalWrite(BUZZPIN, LOW);
  digitalWrite(ALARMPIN, LOW);
  digitalWrite(GRENADEPIN, LOW);
  showTime(RELOJ);
  ENDGAME = false;
  WIN = false;
  CUTTED_WIRE[0] = false;
  CUTTED_WIRE[1] = false;
  CUTTED_WIRE[2] = false;
  CUTTED_WIRE[3] = false;
}

void loop() {
  setup2();
  while (!ENDGAME)
    menu();
}


