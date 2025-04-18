// - Domination
// Device acts as a control point or base station.
// Base starts in neutral state when game begins.
// Teams compete to capture and hold the base.
// When captured, base adds points to controlling team.
// If accelerometer enabled, excessive movement neutralizes the base.
void domination() {
  unsigned long timer = 0;
  unsigned long points[2] = {0, 0};
  bool dominator[2] = {false, false};

  // For accelerometer detection
  float prevAccel = 0;
  bool firstAccelRead = true; // Flag for first read
  unsigned long lastAccelRead = 0; // Throttle accelerometer reading
  unsigned long neutralMsgEndTime = 0; // Timer for clearing "BASE NEUTRALIZED" message

  lcd.setCursor(0, 0);
  lcd.print(RED_TEAM);
  lcd.print("  ");
  lcd.print(GREEN_TEAM);
  lcd.setCursor(0, 1);
  lcd.print("000000    000000");

  // Show initial neutral status
  updateDominationStatus(dominator);

  unsigned long game_counter = millis();
  drawEmptyProgressBar(2);
  int lastPercentage = 0;
  int millisCounter = 0; // Used for point scoring interval

  while (timer < GAME_CLOCK) {

    // Check for shock detection if enabled (read ~10 times/sec)
    if (bSHOCK && accel.available() && (millis() - lastAccelRead > 100)) {
      lastAccelRead = millis();
      accel.read();
      // Calculate magnitude: sqrt(x^2 + y^2 + z^2)
      float currentAccel = sqrt(pow(accel.cx, 2) + pow(accel.cy, 2) + pow(accel.cz, 2));

      if (!firstAccelRead) { // Don't calculate delta on the very first read
          float delta = abs(currentAccel - prevAccel);
          // Check delta against sensitivity (scaled appropriately)
          if (delta > (SHOCK_SENSITIVITY / 10.0)) { // Consistent variable name
              // If movement detected and base is currently captured, neutralize it
              if (dominator[0] || dominator[1]) {
                  dominator[0] = false;
                  dominator[1] = false;
                  updateDominationStatus(dominator); // Update display to NEUTRAL

                  // Give feedback
                  if (bBUZZ) {
                      tone(BUZZPIN, 400, 500); // Neutralized tone
                  }

                  lcdClearRow(3); // Clear button progress row before showing message
                  lcd.setCursor(0, 3);
                  lcd.print(F("BASE NEUTRALIZED!")); // Use F() macro
                  neutralMsgEndTime = millis() + 2000; // Show message for 2 seconds
              }
          }
      }
      prevAccel = currentAccel; // Store current reading for next comparison
      firstAccelRead = false; // Clear flag after first read
    }

    // Clear "BASE NEUTRALIZED" message after timeout
    if (neutralMsgEndTime > 0 && millis() >= neutralMsgEndTime) {
        lcdClearRow(3); // Clear message row
        neutralMsgEndTime = 0; // Reset timer
        // No need to redraw button prompt here, pushedButton handles its own display
    }


    if (countMillis(10, game_counter)) {
      timer++;
      millisCounter++;
      showTime(GAME_CLOCK - timer);
      int x = calculatePercentage(timer, GAME_CLOCK);
      if(lastPercentage != x){
        drawPercentage(16, 2, x);
        int from = percentageBarPosition(lastPercentage, 30);
        int to = percentageBarPosition(x, 30);
        if(from != to) drawProgressBar(from, to, 30, 0, 2);
        lastPercentage = x;
      }
      // Score points every second (100 * 10ms = 1000ms)
      if (millisCounter >= 100) {
        millisCounter = 0;
        // Cap points at 999999 to prevent overflow on display
        if (dominator[0] && points[0] < 999999) {
          points[0]++;
          char buffer[7];
          sprintf(buffer,"%06lu",points[0]); // Use %lu for unsigned long
          lcd.setCursor(0, 1);
          lcd.print(buffer);
        }
        if (dominator[1] && points[1] < 999999) {
          points[1]++;
          char buffer[7];
          sprintf(buffer,"%06lu",points[1]); // Use %lu for unsigned long
          lcd.setCursor(10, 1);
          lcd.print(buffer);
        }
      }
    }

    // Call buzzing unconditionally
    buzzing();

    // Check buttons only if neutral message is not being displayed
    if (neutralMsgEndTime == 0) {
        bool redHeld = pushedButton(RED_BTN, false, 3);
        bool greenHeld = pushedButton(GREEN_BTN, false, 3);

        // If Red holds button and base is Green OR Neutral -> Capture Red
        if (redHeld && (dominator[1] || (!dominator[0] && !dominator[1]))) {
            if (!dominator[0]) { // Only update if not already Red
                dominator[0] = true;
                dominator[1] = false;
                updateDominationStatus(dominator);
            }
        }
        // If Green holds button and base is Red OR Neutral -> Capture Green
        else if (greenHeld && (dominator[0] || (!dominator[0] && !dominator[1]))) {
             if (!dominator[1]) { // Only update if not already Green
                dominator[0] = false;
                dominator[1] = true;
                updateDominationStatus(dominator);
            }
        }
        // If Red button is released while Green is captured -> Neutralize
        else if (!boolRead(RED_BTN) && dominator[1] && lastStateButton) {
             // This case seems unlikely with current pushedButton logic resetting state on release
             // Consider if neutralization should happen instantly on opposite button press instead of hold
        }
         // If Green button is released while Red is captured -> Neutralize
        else if (!boolRead(GREEN_BTN) && dominator[0] && lastStateButton) {
             // Similar to above
        }
    }

  } // end while

  // Game Over
  lcdClearRow(2); // Clear progress bar
  lcdClearRow(3); // Clear button prompt/message row
  if (points[0] == points[1]) {
    lcd.setCursor(0, 3);
    lcd.print(DRAW);
  }
  else {
      winMessage((points[1] > points[0]), 3); // Display winner on row 3
  }
  alarm(true); // Sound alarm at end of game
}

// Helper function to update the domination status display
void updateDominationStatus(bool dominator[2]) {
  // Display status on row 0, overwriting team names temporarily if needed
  lcd.setCursor(0, 0);
  if (dominator[0]) {
    lcd.print(F("<<< RED CAPTURED >>>")); // Use F() macro
    if (bBUZZ) {
      tone(BUZZPIN, 600, 200); // Red capture tone
    }
  }
  else if (dominator[1]) {
    lcd.print(F("<<< GREEN CAPTURED >>>")); // Use F() macro
    if (bBUZZ) {
      tone(BUZZPIN, 800, 200); // Green capture tone
    }
  }
  else {
    // Restore team names when neutral
    lcd.print(RED_TEAM);
    lcd.print("       "); // Clear middle
    lcd.print(GREEN_TEAM);
    // Optional: Neutral tone?
    // if (bBUZZ) { tone(BUZZPIN, 500, 100); }
  }
  lcdClearRow(3); // Clear button progress row when status changes
}
