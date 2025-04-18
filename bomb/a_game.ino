// Functions used by different game modes

bool boolRead(int n) {
  if (digitalRead(n) == HIGH) return true;
  else return false;
}

int calculatePercentage(unsigned long passedTime, unsigned long maxTime) {
  return (int)((passedTime * 100) / maxTime);
}

int percentageBarPosition(int percentage, int barLength) {
  return (int)((percentage * barLength) / 100);
}

bool countMillis(int t, unsigned long &counted) {
  if ((millis() - counted) >= t) {
    counted = millis();
    return true;
  }
  return false;
}

bool checkPass(String &p) {
  char key = keypad.getKey();
  if (key) {
    beep();
    switch (key) {
      case '#':
        if (p == PASS) {
          p = "";
          return true;
        }
        break;
      case '*':
        p = "";
        lcdClearRow(1);
        break;
      default:
        if (p.length() < 20) {
          p += key;
          lcdClearRow(1);
          lcd.setCursor(0, 1);
          for (int i = 0; i < p.length(); i++) lcd.print('*');
        }
        break;
    }
  }
  return false;
}

bool checkWire(unsigned long &timer, bool cuttedWire[]) {
  for (int i = 0; i < 4; i++) {
    if (boolRead(WIRE[i]) && !cuttedWire[i]) {
      cuttedWire[i] = true;
      beep();
      switch (tWIRE[i]) {
        case 1: //-10 sec penalty: Increase elapsed time
          timer += 1000;
          break;
        case 2: //Correct wire: Stop bomb (handled by return value)
          return true;
          break;
        case 3:
          // Do nothing ;
          break;
        case 4: //BOOM!!! Set elapsed time to maximum (or beyond)
          // Setting to BOMB_CLOCK or GAME_CLOCK depends on context,
          // setting high value ensures game ends. 36000000 = 10 hours
          timer = 36000000;
          break;
      }
    }
  }
  return false;
}

bool checkNFC(unsigned long &timer) {
  DEBUG_PRINTLN(F("checkNFC: Checking for NFC card..."));
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    DEBUG_PRINTLN(F("checkNFC: Card detected!"));
    byte readData[18]; // Buffer needs 18 for read function internal use, even if reading 16
    readNFC(readData); // readNFC now reads 16 bytes into data
    mfr_halt();
    DEBUG_PRINT(F("checkNFC: Card UID:"));
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      DEBUG_PRINT(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX); // Use Serial.print directly for HEX format
    }
    DEBUG_PRINTLN("");
    DEBUG_PRINTF("checkNFC: Card Data (Uses: %d, Type: %d, Time: %lu)\n", readData[1], readData[0], (unsigned long)readData[2] * 360000 + (unsigned long)readData[3] * 6000 + (unsigned long)readData[4] * 100 + readData[5]);


    if (readData[1] > 0) { // Check uses remaining
      DEBUG_PRINTLN(F("checkNFC: Card has uses remaining."));
      beep();
      byte currentUses = readData[1];
      if (currentUses != 'A') { // 'A' means infinite
         readData[1]--; // Decrement uses
         DEBUG_PRINTF("checkNFC: Decrementing uses. New uses: %d. Writing back to card...\n", readData[1]);
         // Write updated data back to card only if uses changed
         writeNFC(readData);
      } else {
         DEBUG_PRINTLN(F("checkNFC: Infinite uses ('A'). Not writing back."));
      }

      // Process card effect based on type (readData[0])
      if (readData[0] == 1) { // Type 1: Stop Bomb
        DEBUG_PRINTLN(F("checkNFC: Card Type 1 (Stop Bomb). Returning true."));
        return true;
      }
      else {
        // Calculate time effect 'r' in centiseconds
        unsigned long r = (unsigned long)readData[2] * 360000 + (unsigned long)readData[3] * 6000 + (unsigned long)readData[4] * 100 + readData[5];
        DEBUG_PRINTF("checkNFC: Card Type %d. Time effect (r): %lu cs. Current elapsed time (timer): %lu cs.\n", readData[0], r, timer);
        if (readData[0] == 2) { // Type 2: Add time (bonus) -> Decrease elapsed time
          if (timer >= r) { // Prevent underflow
             timer -= r;
             DEBUG_PRINTF("checkNFC: Decreasing elapsed time. New timer: %lu cs.\n", timer);
          } else {
             timer = 0; // Set elapsed time to 0 if bonus is larger than current elapsed time
             DEBUG_PRINTLN(F("checkNFC: Bonus larger than elapsed time. Setting timer to 0."));
          }
        }
        else if (readData[0] == 3) { // Type 3: Subtract time (penalty) -> Increase elapsed time
          timer += r;
          DEBUG_PRINTF("checkNFC: Increasing elapsed time. New timer: %lu cs.\n", timer);
          // No upper bound check needed here, main game loop handles max time
        }
      }
    } else {
        DEBUG_PRINTLN(F("checkNFC: Card has no uses left."));
        // Card has no uses left
        // Optional: Add feedback like a different beep
        tone(BUZZPIN, 100, 200); // Low beep for empty card
    }
  } else {
     // DEBUG_PRINTLN(F("checkNFC: No card found.")); // Optional: uncomment if you want logs even when no card is present
  }
  return false;
}

void waitFor(int s) {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(GAME_START_IN);
  unsigned long t = millis();
  if (((int)s / 10) > 0) {
    customNum(((int)s / 10), 7, 1);
    customNum(((int)s % 10), 10, 1);
  }
  else customNum(((int)s % 10), 8, 1);
  while (s != 0) {
    if (millis() - t > 1000) {
      t = millis();
      s--;
      if (((int)s / 10) > 0) {
        customNum(((int)s / 10), 7, 1);
        customNum(((int)s % 10), 10, 1);
      }
      else {
        if (s % 10 == 9) {
          lcdClear(7, 1, 8, 2);
          lcdClear(11, 1, 13, 2);
        }
        customNum(((int)s % 10), 8, 1);
      }
    }
  }
  lcd.clear();
}

void showTime( unsigned long t) {
  //const byte NUM[] = {B11111110,B10110000,B11101101,B11111001,B10110011,B11011011,B11011111,B11110000,B11111111,B11111011};
  const byte POINT = B10000000;
  const byte NUM[] = {B01111110, B00110000, B01101101, B01111001, B00110011, B01011011, B01011111, B01110000, B01111111, B01111011};
  unsigned long h = t / 360000;
  unsigned long m = ((t / 100) % 3600) / 60;
  unsigned long s = ((t / 100) % 3600) % 60;
  unsigned long cs = t % 100;

  ld.write(8, NUM[(int)h / 10] );
  ld.write(7, NUM[(int)h % 10] + (POINT /** (cs / 10 % 2)*/));
  ld.write(6, NUM[(int)m / 10] );
  ld.write(5, NUM[(int)m % 10] + (POINT /** (cs / 10 % 2)*/));
  ld.write(4, NUM[(int)s / 10] );
  ld.write(3, NUM[(int)s % 10] + (POINT /** (cs / 10 % 2)*/));
  ld.write(2, NUM[(int)cs / 10] );
  ld.write(1, NUM[(int)cs % 10] );

  if (millis() / 500 % 2) ld.setBright(1);
  else ld.setBright(15);
}

void drawPercentage(int column, int row, int percentage) {
  lcd.setCursor(column, row);
  if (percentage < 100) lcd.print(" ");
  if (percentage < 10) lcd.print(" ");
  lcd.print(percentage);
  lcd.print("%");
}



void drawProgressBar(int from, int to, int maxPercentage, int column, int row) {
  byte bar10[] = {0x0F, 0x18, 0x13, 0x17, 0x17, 0x13, 0x18, 0x0F}; //left end full
  byte bar11[] = {0x1F, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x1F}; //center half
  byte bar12[] = {0x1F, 0x00, 0x1B, 0x1B, 0x1B, 0x1B, 0x00, 0x1F}; //center full
  byte bar13[] = {0x1E, 0x03, 0x19, 0x1D, 0x1D, 0x19, 0x03, 0x1E}; //right end full

  lcd.createChar(3, bar10);
  lcd.createChar(4, bar11);
  lcd.createChar(5, bar12);
  lcd.createChar(6, bar13);

  for (int x = from; x <= to; x++) {
    if (x == 1) {
      lcd.setCursor(column, row);
      lcd.write(3);
    }
    else if (x == maxPercentage) {
      lcd.setCursor(column + ((int)maxPercentage / 2), row);
      lcd.write(6);
    }
    else if (((int)x / 2) != 0) {
      lcd.setCursor(column + ((int)x / 2), row);
      if (x % 2 == 1) lcd.write(5);
      else lcd.write(4);
    }
  }
}

void drawEmptyProgressBar(int row) {
  //empty bar
  byte bar00[] = {0x0F, 0x18, 0x10, 0x10, 0x10, 0x10, 0x18, 0x0F}; //left end
  byte bar01[] = {0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}; //center
  byte bar02[] = {0x1E, 0x03, 0x01, 0x01, 0x01, 0x01, 0x03, 0x1E}; //right end

  lcd.createChar(0, bar00);
  lcd.createChar(1, bar01);
  lcd.createChar(2, bar02);

  lcd.setCursor(0, row);
  lcd.write(0);
  for (int i = 1; i < 15; i++) {
    lcd.setCursor(i, row);
    lcd.write(1);
  }
  lcd.setCursor(15, row);
  lcd.write(2);
  lcd.print("  0%");
}

void printActiveGames(bool bp, bool bw, bool bk, int row, bool isGreen) {
  lcdClearRow(row);
  lcd.setCursor(0, row);
  if (bp) lcd.print(GAME_PASS);
  if (bw) {
    if (bp) lcd.print(";");
    lcd.print(GAME_WIRE);
  }
  if (bk) {
    if (bp || bw) lcd.print(";");
    lcd.print(GAME_NFC);
  }

  if (!bp && !bw && !bk) {
    if (isGreen) lcd.print(PRESS_GREEN_BUTTON);
    else lcd.print(PRESS_RED_BUTTON);
  }
}

void customNum(int num, int column, int row) {
  byte LT[8] = {0x07, 0x0F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
  byte UB[8] = {0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00};
  byte RT[8] = {0x1C, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
  byte LL[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0F, 0x07};
  byte LB[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F};
  byte LR[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1E, 0x1C};
  byte MB[8] = {0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x1F};
  byte block[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
  lcd.createChar(0, LT);
  lcd.createChar(1, UB);
  lcd.createChar(2, RT);
  lcd.createChar(3, LL);
  lcd.createChar(4, LB);
  lcd.createChar(5, LR);
  lcd.createChar(6, MB);
  lcd.createChar(7, block);

  lcd.setCursor(column, row);

  switch (num) {
    case 0:
      lcd.write(0);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(column, row + 1);
      lcd.write(3);
      lcd.write(4);
      lcd.write(5);
      break;
    case 1:
      lcd.write(1);
      lcd.write(2);
      lcd.print(" ");
      lcd.setCursor(column, row + 1);
      lcd.write(4);
      lcd.write(7);
      lcd.write(4);
      break;
    case 2:
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(column, row + 1);
      lcd.write(3);
      lcd.write(4);
      lcd.write(4);
      break;
    case 3:
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(column, row + 1);
      lcd.write(4);
      lcd.write(4);
      lcd.write(5);
      break;
    case 4:
      lcd.write(3);
      lcd.write(4);
      lcd.write(7);
      lcd.setCursor(column, row + 1);
      lcd.print(" ");
      lcd.print(" ");
      lcd.write(7);
      break;
    case 5:
      lcd.write(3);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(column, row + 1);
      lcd.write(4);
      lcd.write(4);
      lcd.write(5);
      break;
    case 6:
      lcd.write(0);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(column, row + 1);
      lcd.write(3);
      lcd.write(4);
      lcd.write(5);
      break;
    case 7:
      lcd.write(1);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(column, row + 1);
      lcd.print(" ");
      lcd.print(" ");
      lcd.write(7);
      break;
    case 8:
      lcd.write(0);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(column, row + 1);
      lcd.write(3);
      lcd.write(4);
      lcd.write(5);
      break;
    case 9:
      lcd.write(0);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(column, row + 1);
      lcd.print(" ");
      lcd.print(" ");
      lcd.write(7);
      break;
  }
}

unsigned long TIME_BUZZING = millis();

void buzzing() {
  if (bBUZZ) {
    unsigned long time = millis();
    if (time - TIME_BUZZING >= 1000) {  // Check if 1 second has passed
      TIME_BUZZING = time;              // Update the last buzz time
      tone(BUZZPIN, 220, 100);          // Generate the buzz sound
    }
  }
}

void beep() {
  tone(BUZZPIN, 500, 100);
}

void alarm(bool b) {
  if (bALARM && b) digitalWrite(ALARMPIN, HIGH);
  if(!b) digitalWrite(ALARMPIN, LOW);
}

void grenade() {
  if (bGRENADE) digitalWrite(GRENADEPIN, HIGH);
  delay(1000);
  digitalWrite(GRENADEPIN, LOW);
}

void winMessage(bool b, int row) {
  lcd.setCursor(0, row);
  if (b) lcd.print(WIN_MESSAGE_GREEN);
  else lcd.print(WIN_MESSAGE_RED);
}

// Global progress bar tracking variables that can be reused across game modes
int lastPercentage = 0;
int pushedButton_timer = 0;
unsigned long pushedButton_counter = millis();
int pushedButton_lastPercentage = 0;
bool lastStateButton = false;

bool pushedButton(int button, bool hilow, int row) {
  if (boolRead(button) == hilow) {
    lastStateButton = true;
    // Use ARM_DISARM_TIME (defined in bomb.ino) instead of undefined TIME_ARMDES
    if (ARM_DISARM_TIME == 0) return true; // Instant if time is 0
    if (pushedButton_timer == 0) drawEmptyProgressBar(row);
    // Use ARM_DISARM_TIME for calculation
    if (countMillis(100, pushedButton_counter)) { // Check every 100ms (0.1s)
        // Increment counter towards ARM_DISARM_TIME * 10 (since ARM_DISARM_TIME is in seconds)
        if (pushedButton_timer < (ARM_DISARM_TIME * 10)) {
             pushedButton_timer++;
        }
    }
    // Calculate percentage based on ARM_DISARM_TIME in tenths of seconds
    int x = calculatePercentage(pushedButton_timer, ARM_DISARM_TIME * 10);
    if(pushedButton_lastPercentage != x){
      drawPercentage(16, row, x);
      int from = percentageBarPosition(pushedButton_lastPercentage, 30);
      int to = percentageBarPosition(x, 30);
      if(from != to) drawProgressBar(from, to, 30, 0, row);
      pushedButton_lastPercentage = x;
    }
    // Check if button held long enough (compare with ARM_DISARM_TIME in tenths of seconds)
    if (pushedButton_timer >= (ARM_DISARM_TIME * 10)) {
      pushedButton_timer = 0;
      pushedButton_lastPercentage = 0; // Reset percentage too
      lastStateButton = false; // Reset state as action is complete
      lcdClearRow(row); // Clear progress bar row
      return true;
    }
  }
  else if(lastStateButton) { // Button released before completion
    pushedButton_counter = millis();
    pushedButton_timer = 0;
    pushedButton_lastPercentage = 0;
    lcdClearRow(row); // Clear progress bar row
    lastStateButton = false;
  }
  return false;
}

// Make lcdClearRow and lcdClear the main implementations to avoid duplication
void lcdClearRow(int row) {
  lcd.setCursor(0, row);
  lcd.print("                    ");
}

void lcdClear(int col, int row, int endCol, int endRow) {
  for (int r = row; r <= endRow; r++)
    for (int c = col; c <= endCol; c++) {
      lcd.setCursor(c, r);
      lcd.print(" ");
    }
}
