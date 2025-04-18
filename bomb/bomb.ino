#include <SparkFun_MMA8452Q.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DigitLedDisplay.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <stdio.h> // Required for vsnprintf
#include <stdarg.h> // Required for va_list, va_start, va_end

// --- Debugging ---
#define DEBUG // Comment this line out to disable Serial debugging
#ifdef DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  // Helper function for printf-style debugging
  void debugPrintf(const char *fmt, ...) {
    char buf[128]; // Create a buffer for the formatted string
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args); // Format the string into the buffer
    va_end(args);
    Serial.print(buf); // Print the buffer
  }
  #define DEBUG_PRINTF(fmt, ...) debugPrintf(fmt, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(fmt, ...) // Keep this empty for non-debug builds
#endif
// --- End Debugging ---

//Digit led display pins
#define DIN 30
#define LOAD 32
#define CLK 34

#define BUZZPIN A15
#define ALARMPIN 40
#define GRENADEPIN 41

//KEYPINS
// ROWS
#define KEY0 25
#define KEY1 27
#define KEY2 29
#define KEY3 31
// COLUMNS
#define KEY4 33
#define KEY5 35
#define KEY6 37
#define KEY7 39

// Wire pins
const int WIRE[4] = {14, 15, 16, 17};

// Button pins
#define RED_BTN 18
#define GREEN_BTN 19

const byte rowsCount = 4;
const byte columnsCount = 4; // renamed from columsCount
char keys[rowsCount][columnsCount] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[rowsCount] = { KEY0, KEY1, KEY2, KEY3 };
byte columnPins[columnsCount] = { KEY4, KEY5, KEY6, KEY7 };

DigitLedDisplay ld = DigitLedDisplay(DIN, LOAD, CLK);
Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rowsCount, columnsCount);
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define RST_PIN 49
#define SS_PIN 53

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key nfcKey;

MMA8452Q accel;

//----------------------------------TEXTS--------------------------------------------
const String MAIN_MENU[] = { "Menu", "1.-Start game", "2.-Settings", ""};
const String GAME_MENU[] = {"1.-Counterstrike", "2.-Assault", "3.-Domination", "4.-Explosive"};
const String MENU_CONF[] = {"1.-Time", "2.-Lock mechanism", "3.-Other options", "4.-Test"};
const String MENU_TIME[] = {"1.-Game time", "2.-Bomb time", "3.-Arm/Disarm time", "4.-Start time"};
const String MENU_TIME2[] = {"Current time", "", "1.-Change", "0.-Back"};
const String MENU_LOCKS[] = {"1.-Active locks", "2.-Password", "3.-Wires", "4.-NFC Keys"}; // was MENU_LOCKERS
const String MENU_ACTIVE_LOCKS[] = {"1.-Password", "2.-Wires", "3.-NFC Keys", ""}; // was MENU_ACTIVE_LOCKERS
const String MENU_PASS[] = {"Current password", "", "1.-Change", "0.-Back"};
const String NEW_PASS[] = {"Enter new", "password:", "*Delete #Confirm", ""};
const String MENU_WIRE[] = {"Wire function", "1.-W1 =    2.-W2 =  ", "3.-W3 =    4.-W4 =", ""};
const String FUNCTION_WIRE[] = {"1.-Reduce time", "2.-Stop bomb", "3.-Do nothing", "4.-Explode"};
const String MENU_NFC[] = {"1.-Card options", "2.-Read card", "3.-Write card", ""};
const String MENU_NFC_CONF[] = {"1.-Type", "2.-Uses", "3.-Time", ""};
const String MENU_NFC_TYPE[] = {"1.-Stop Bomb", "2.-Add time", "3.-Subtract time", ""};
const String MENU_NFC_USES[] = {"Number of uses", "from 1 to 9, or A for", "infinite uses", ""};
const String MENU_OTHER_OPT[] = {"1.-Sound", "2.-Alarm", "3.-Grenade", "4.-Shock sensor"};
const String MENU_SHOCK[] = {"1.-Shock sensor", "2.-Adjust sensitivity", "Current sensitivity:", ""};
const String MENU_SHOCK2[] = {"1.-Manual adjust", "2.-Shock adjust", "Current sensitivity:", ""};
const String WAIT_FOR_SHOCK = {"Hit the device"};
const String NFC_CARD_ON_READER = "Place NFC card on reader...";
const String WIN_MESSAGE_RED = "    RED TEAM WINS    ";
const String WIN_MESSAGE_GREEN = "   GREEN TEAM WINS   ";
const String DRAW = "DRAW";
const String PRESS_RED_BUTTON = "Press red button"; 
const String PRESS_GREEN_BUTTON = "Press green button";
const String BOMB_ACTIVE = "BOMB ARMED";
const String BOMB_UNACTIVE = "BOMB DISARMED";
const String INTRO_TIME = "Enter time:";
const String GAME_PASS = "PASS";
const String GAME_WIRE = "WIRE";
const String GAME_NFC = "NFC";
const String GAME_START_IN = "Game starts in";
const String RED_TEAM = "Red team";
const String GREEN_TEAM = "Green team";
const String INTRO2NUM[] = {"Enter a value", "of 2 digits:", "", ""};
//------------------------------------------------------------------------------------

//-----------------------GLOBAL BOMB VARIABLES--------------------------------------
bool bBUZZ = true;
bool bALARM = true;
bool bGRENADE = true;

bool bSHOCK = false;
byte SHOCK_SENSITIVITY = 75; // Variable name is consistent now

bool bPASS = true; // Password lock
String PASS = "123ABC"; // Password

bool bWIRE = false; // Wire lock
int tWIRE[4] = {2, 3, 3, 3}; // Wire effect: 1=Reduce time 2=Stop bomb 3=Do nothing 4=Explode!

bool bNFC = false; // NFC lock

unsigned long GAME_CLOCK = 60000; // Game time in centiseconds (10 min)
unsigned long BOMB_CLOCK = 30000; // Bomb time in centiseconds (5 min)
int ARM_DISARM_TIME = 10; // Bomb arm/disarm time in seconds
int INIT_TIME = 10; // Game start time in seconds
void showTime(unsigned long t);
void menu();

//------------------------------------------------------------------------------------



void setup() {
  Serial.begin(9600);
  DEBUG_PRINTLN(F("Serial Debug Enabled. Initializing..."));
  Wire.begin();
  accel.begin();
  lcd.init();
  lcd.backlight();
  ld.setDigitLimit(8);//8 digits
  SPI.begin();
  mfrc522.PCD_Init();
  for (byte i = 0; i < 6; i++) {
    nfcKey.keyByte[i] = 0xFF;
  }
  pinMode(GRENADEPIN, OUTPUT);
  pinMode(ALARMPIN, OUTPUT);
  pinMode(BUZZPIN, OUTPUT);
  pinMode(WIRE[0], INPUT_PULLUP);
  pinMode(WIRE[1], INPUT_PULLUP);
  pinMode(WIRE[2], INPUT_PULLUP);
  pinMode(WIRE[3], INPUT_PULLUP);
  pinMode(RED_BTN, INPUT_PULLUP);
  pinMode(GREEN_BTN, INPUT_PULLUP);

// Call setup2() once during initialization

//  DEBUG_PRINTLN(F("Setup complete."));
}

void setup2() {
  ld.setBright(15); //1-15
  digitalWrite(BUZZPIN, LOW);
  digitalWrite(ALARMPIN, LOW);
  digitalWrite(GRENADEPIN, LOW);
  showTime(BOMB_CLOCK);
}

void loop() {
  setup2();
  menu();
}
