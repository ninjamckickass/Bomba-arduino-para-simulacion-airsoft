// Menu system functions

void menu() {
  printScreen(MAIN_MENU);
  drawBomb(17, 0);
  switch (validOption(1, 2)) {
    case 1:
      menuGame();
      break;
    case 2:
      menuConfig();
      break;
  }
}

void menuGame() {
  int opt;
  do {
    printScreen(GAME_MENU);
    drawBomb(17, 0);
    opt = validOption(0, 4);
    if (opt != 0) {
      waitFor(INIT_TIME);
      alarm(true);
      delay(1000);
      alarm(false);
    }
    switch (opt) {
      case 1: //1.-Counterstrike
        counterstrike();
        break;
      case 2: //2.-Assault
        assault();
        break;
      case 3: //3.-Domination
        domination();
        break;
      case 4: //4.-Explosive
        explosive();
        break;
    }
    if (opt != 0) {
      keypad.waitForKey();
      opt = 0;
    }
  } while (opt != 0);
}

void menuConfig() {
  //1.-Time", "2.-Lock mechanism", "3.-Other options", "4.-Test
  int opt;
  do {
    printScreen(MENU_CONF);
    opt = validOption(0, 4);
    switch (opt) {
      case 1:
        menuTime();
        break;
      case 2:
        menuLocks();
        break;
      case 3:
        menuOtherOptions();
        break;
      case 4:
        test();
        break;
    }
  } while (opt != 0);
}

void menuTime() {
  //1.-Game time", "2.-Bomb time", "3.-Arm/disarm time", "4.-Start time
  int opt;
  do {
    printScreen(MENU_TIME);
    opt = validOption(0, 4);
    switch (opt) {
      case 1:
        while (opt != 0) {
          printScreen(MENU_TIME2);
          printLong(GAME_CLOCK, 0, 1);
          opt = validOption(0, 1);
          if (opt == 1) {
            int auxR[4];
            GAME_CLOCK = readTime(auxR);
          }
        }
        opt = -1;
        break;
      case 2:
        while (opt != 0) {
          printScreen(MENU_TIME2);
          printLong(BOMB_CLOCK, 0, 1);
          opt = validOption(0, 1);
          if (opt == 1) {
            int auxR[4];
            BOMB_CLOCK = readTime(auxR);
          }
        }
        opt = -1;
        break;
      case 3:
        while (opt != 0) {
          printScreen(MENU_TIME2);
          lcd.setCursor(0, 1);
          lcd.print(ARM_DISARM_TIME);
          lcd.print(" seconds");
          opt = validOption(0, 1);
          if (opt == 1) ARM_DISARM_TIME = readSeconds();
        }
        opt = -1;
        break;
      case 4:
        while (opt != 0) {
          printScreen(MENU_TIME2);
          lcd.setCursor(0, 1);
          lcd.print(INIT_TIME);
          lcd.print(" seconds");
          opt = validOption(0, 1);
          if (opt == 1) INIT_TIME = readSeconds();
        }
        opt = -1;
        break;
    }
  } while (opt != 0);
}

void menuLocks() {//1.-Active locks", "2.-Password", "3.-Wires", "4.-NFC keys
  int opt;
  do {
    printScreen(MENU_LOCKS);
    opt = validOption(0, 4);
    switch (opt) {
      case 1:
        menuActiveLocks();
        break;
      case 2:
        while (opt != 0) {
          printScreen(MENU_PASS);
          lcd.setCursor(0, 1);
          lcd.print(PASS);
          opt = validOption(0, 1);
          if (opt == 1) newPass();
        }
        opt = -1;
        break;
      case 3:
        while (opt != 0) {
          printScreen(MENU_WIRE);
          lcd.setCursor(8, 1);
          lcd.print(tWIRE[0]);
          lcd.setCursor(19, 1);
          lcd.print(tWIRE[1]);
          lcd.setCursor(8, 2);
          lcd.print(tWIRE[2]);
          lcd.setCursor(19, 2);
          lcd.print(tWIRE[3]);

          opt = validOption(0, 4);
          if (opt != 0) {
            printScreen(FUNCTION_WIRE);
            tWIRE[opt - 1] = validOption(1, 4);
          }
        }
        opt = -1;
        break;
      case 4:
        menuNfc();
        break;
    }
  } while (opt != 0);
}

void menuNfc() {
  int opt;
  byte data[16] = {1, 1, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  //data[0] type
  //data[1] number of uses
  //data[2]:data[3]:data[4]:data[5] time hh:mm:ss:ds
  do {
    printScreen(MENU_NFC);
    printData(data);
    opt = validOption(0, 3);
    switch (opt) {
      case 1:
        while (opt != 0) {//1.-Type", "2.-Uses", "3.-Time", ""
          printScreen(MENU_NFC_CONF);
          printData(data);
          opt = validOption(0, 3);
          switch (opt) {
            case 1:
              printScreen(MENU_NFC_TYPE);
              data[0] = validOption(1, 3);
              break;
            case 2:
              printScreen(MENU_NFC_USES);
              data[1] = validOption(1, 'A');// A = infinite uses
              break;
            case 3:
              int auxR[4];
              readTime(auxR);
              for (int i = 0; i < 4; i++)
                data[i + 2] = auxR[i];
              break;
          }
        }
        opt = -1;
        break;
      case 2:
        waitForNewNFC();
        byte readData[18];
        readNFC(readData);
        for (int i = 0; i < 16; i++)
          data[i] = readData[i];
        mfr_halt();
        break;
      case 3:
        waitForNewNFC();
        writeNFC(data);
        mfr_halt();
        break;
    }
  } while (opt != 0);
  opt = -1;
}

void menuActiveLocks() {
  int opt;
  printScreen(MENU_ACTIVE_LOCKS);
  do {
    check(MENU_ACTIVE_LOCKS[0].length() + 1, 0, bPASS);
    check(MENU_ACTIVE_LOCKS[1].length() + 1, 1, bWIRE);
    check(MENU_ACTIVE_LOCKS[2].length() + 1, 2, bNFC);
    opt = validOption(0, 3);
    if (opt == 1) bPASS = !bPASS;//Password
    if (opt == 2) bWIRE = !bWIRE;//Wires
    if (opt == 3) bNFC = !bNFC;//NFC key
  } while (opt != 0); //0.-Back
}

void menuOtherOptions() {
  int opt;
  printScreen(MENU_OTHER_OPT);
  do {
    check(MENU_OTHER_OPT[0].length() + 1, 0, bBUZZ);
    check(MENU_OTHER_OPT[1].length() + 1, 1, bALARM);
    check(MENU_OTHER_OPT[2].length() + 1, 2, bGRENADE);
    opt = validOption(0, 4);
    if (opt == 1) bBUZZ = !bBUZZ;
    if (opt == 2) bALARM = !bALARM;
    if (opt == 3) bGRENADE = !bGRENADE;
    if (opt == 4) menuShock();
  } while (opt != 0);
}

void menuShock() {
  int opt;
  do {
    printScreen(MENU_SHOCK);
    check(MENU_SHOCK[0].length() + 1, 0, bSHOCK);
    lcd.setCursor(0, 3);
    lcd.print(SHOCK_SENSITIVITY); // Changed from SHOCK_SENSIBILITY
    opt = validOption(0, 2);
    if (opt == 1) bSHOCK = !bSHOCK;
    else if (opt == 2) {
      do {
        printScreen(MENU_SHOCK2);
        lcd.setCursor(0, 3);
        lcd.print(SHOCK_SENSITIVITY); // Changed from SHOCK_SENSIBILITY
        opt = validOption(0, 2);
        switch (opt) {
          case 1:
            //manual adjustment SHOCK_SENSITIVITY != 0
            printScreen(INTRO2NUM);
            opt = validOption(0, 9);
            lcd.setCursor(2, 0);
            lcd.print(opt);
            SHOCK_SENSITIVITY = opt * 10; // Changed from SHOCK_SENSIBILITY
            if (opt == 0) opt = validOption(1, 9);
            else opt = validOption(0, 9);
            SHOCK_SENSITIVITY += opt; // Changed from SHOCK_SENSIBILITY
            lcd.print(opt);
            opt = -1;
            break;
          case 2:
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(WAIT_FOR_SHOCK);
            byte s = 0;
            while (s == 0)
              s = accel.readTap();
            beep();
            SHOCK_SENSITIVITY = s; // Changed from SHOCK_SENSIBILITY
            break;
        }
      } while (opt != 0);
      opt = -1;
    }
  } while (opt != 0);
}

void drawBomb(int col, int fil) {
  byte bo0[] = {0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x04, 0x0C};
  byte bo1[] = {0x00, 0x00, 0x11, 0x0A, 0x13, 0x08, 0x16, 0x05};
  byte bo2[] = {0x01, 0x03, 0x07, 0x0F, 0x0F, 0x1F, 0x1F, 0x1F};
  byte bo3[] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
  byte bo4[] = {0x00, 0x18, 0x1C, 0x1C, 0x1E, 0x1E, 0x1E, 0x1E};
  byte bo5[] = {0x1F, 0x0F, 0x0F, 0x07, 0x01, 0x00, 0x00, 0x00};
  byte bo6[] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00};
  byte bo7[] = {0x1E, 0x1C, 0x1C, 0x18, 0x00, 0x00, 0x00, 0x00};

  lcd.createChar(0, bo0);
  lcd.createChar(1, bo1);
  lcd.createChar(2, bo2);
  lcd.createChar(3, bo3);
  lcd.createChar(4, bo4);
  lcd.createChar(5, bo5);
  lcd.createChar(6, bo6);
  lcd.createChar(7, bo7);

  lcd.setCursor(col, fil);
  lcd.print(" ");
  lcd.write(0);
  lcd.write(1);
  lcd.setCursor(col, fil + 1);
  lcd.write(2);
  lcd.write(3);
  lcd.write(4);
  lcd.setCursor(col, fil + 2);
  lcd.write(5);
  lcd.write(6);
  lcd.write(7);
}

void printData(byte data[]) {
  lcd.setCursor(0, 3);
  lcd.print(data[0] + " ; ");
  if (data[1] != 'A') lcd.print(data[1]);
  else lcd.print("A");
  char buffer[20];
  sprintf(buffer, " ; %02d:%02d:%02d:%02d", data[2], data[3], data[4], data[5]);
  lcd.print(buffer);
  /*
    if (data[2] < 10) lcd.print("0");
    lcd.print(data[2]);
    lcd.print(":");
    if (data[3] < 10) lcd print("0");
    lcd.print(data[3]);
    lcd.print(":");
    if (data[4] < 10) lcd print("0");
    lcd.print(data[4]);
    lcd.print(":");
    if (data[5] < 10) lcd print("0");
    lcd.print(data[5]);*/
}


int validOption(int minOpt, int maxOpt) {
  int opt;
  do {
    opt = keypad.waitForKey() - '0';
    if (opt > 9) opt += '0'; // if it's a letter
  } while (opt < minOpt || opt > maxOpt);
  beep();
  return opt;
}

void printScreen(const String m[]) {
  lcd.clear();
  for (int i = 0; i < 4; i++) {
    lcd.setCursor(0, i);
    lcd.print(m[i]);
  }
}

int readSeconds() {
  int s;
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(INTRO_TIME);
  lcd.setCursor(0, 2);
  s = validOption(0, 9) * 10;
  s += validOption(0, 9);
  return s;
}

unsigned long readTime(int t[4]) {
  const byte POINT = B10000000;
  const byte NUM[] = {B01111110, B00110000, B01101101, B01111001, B00110011, B01011011, B01011111, B01110000, B01111111, B01111011};
  //long h, m, s, cs;
  for (int i = 0; i < 4; i++)
    t[i] = 0;
  ld.clear();
  lcd.setCursor(0, 1);
  lcd.print(INTRO_TIME);
  lcd.setCursor(0, 2);
  lcd.print("  :  :  :  ");
  lcd.setCursor(0, 2);
  int key;
  for (int i = 0; i < 8; i++) {
    int maxDigit = 9;
    // Tens digit for minutes (i=2) or seconds (i=4) must be 0-5
    if (i == 2 || i == 4) {
        maxDigit = 5;
    }
    // Get key ensuring it's within the valid range for the current digit
    do {
        key = keypad.waitForKey() - '0';
    } while (key < 0 || key > maxDigit);
    beep(); // Beep after valid key

    ld.write(8 - i, NUM[key] ); // Display digit on LED
    lcd.print(key); // Display digit on LCD
    t[(int)i / 2] += key;
    if (i % 2 == 0) {
        t[(int)i / 2] *= 10; // Shift tens digit
    } else if (i < 7) { // Don't print ':' after the last digit
        lcd.print(":"); // Print separator
    }
  }
  unsigned long mul_h = 360000,
       mul_m = 6000,
       mul_s = 100;
  unsigned long l = (t[0] * mul_h) + (t[1] * mul_m) + (t[2] * mul_s) + t[3];
  return l;
}

void printLong(unsigned long t, int col, int fil) {
  unsigned int h = t / 360000;
  unsigned int m = (t % 360000) / 6000;
  unsigned int s = ((t % 360000) % 6000) / 100;
  unsigned int cs = t % 100;
  lcd.setCursor(col, fil);

  char buffer[15];
  sprintf(buffer, "%02d:%02d:%02d:%02d", h, m, s, cs);
  lcd.print(buffer);

  /*
    if (h < 10) lcd.print("0");
    lcd.print(h);
    lcd.print(":");
    if (m < 10) lcd print("0");
    lcd.print(m);
    lcd.print(":");
    if (s < 10) lcd print("0");
    lcd.print(s);
    lcd.print(":");
    if (cs < 10) lcd print("0");
    lcd.print(cs);*/
}

void newPass() {
  printScreen(NEW_PASS);
  char auxPass[21]; // Increased size by 1 for null terminator
  char key;
  int i = 0;
  do {
    key = keypad.waitForKey();
    beep();
    if (key == '*' && i != 0) i--;
    // Ensure not to write past buffer boundary (leave space for null terminator)
    else if (key != '#' && key != '*' && i < 20) {
      auxPass[i] = key;
      i++;
    }
    lcdClear(0, 3, 19, 3);
    lcd.setCursor(0, 3); // Set cursor before loop
    for (int i2 = 0; i < i; i2++) {
      // lcd.setCursor(i2, 3); // Setting cursor repeatedly in loop is inefficient
      lcd.print(auxPass[i2]);
    }
  } while (key != '#');
  auxPass[i] = '\0'; // Add null terminator
  if (i == 0) PASS = "";
  else PASS = String(auxPass); // String constructor handles null terminator
}

void check(int col, int fil , bool yesno) {
  byte noCheck[] = {0x00, 0x1F, 0x11, 0x11, 0x11, 0x1F, 0x00, 0x00};
  byte yesCheck[] = {0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00};
  lcd.createChar(0, noCheck);
  lcd.createChar(1, yesCheck);
  lcd.setCursor(col, fil);
  lcd.write((int)yesno);
}
