// - Explosive
// Bomb starts armed with countdown active.
// Game ends when bomb detonates or is disarmed.
// If accelerometer enabled, excessive movement triggers 3-second detonation.

void explosive() {
  unsigned long timer = 0;
  String pass = "";
  bool bp = bPASS,
       bw = bWIRE,
       bk = bNFC;
  bool win = false;
  bool gameOver = false;
  bool cutWires[4];
  for (int i = 0; i < 4; i++)
    cutWires[i] = boolRead(WIRE[i]); // Initialize based on initial state
  drawEmptyProgressBar(3);
  printActiveGames(bp, bw, bk, 0, true);
  unsigned long game_counter = millis();

  // For accelerometer detection
  float prevAccel = 0;
  bool firstAccelRead = true; // Flag for first read
  bool shockDetected = false;
  unsigned long shockTime = 0;
  unsigned long lastAccelRead = 0; // Throttle accelerometer reading

  while (!gameOver) {
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
              if (!shockDetected) {
                  shockDetected = true;
                  shockTime = millis();
                  lcdClearRow(1); // Clear row 1 before printing message
                  lcd.setCursor(0, 1);
                  lcd.print(F("MOVEMENT DETECTED!")); // Use F() macro
              }
          }
      }
      prevAccel = currentAccel; // Store current reading for next comparison
      firstAccelRead = false; // Clear flag after first read

      // If shock was detected 3 seconds ago, detonate
      if (shockDetected && (millis() - shockTime > 3000)) {
          win = false;
          gameOver = true;
          // No break needed here, loop condition will handle exit
      }
    }

    // Only proceed with game logic if not game over due to shock
    if (!gameOver) {
        if (countMillis(10, game_counter)) {
          timer++;
          // Check if bomb exploded BEFORE updating display
          if (timer >= BOMB_CLOCK) {
            timer = BOMB_CLOCK; // Cap timer at max
            showTime(0); // Show 00:00:00:00
            drawPercentage(16, 3, 100);
            drawProgressBar(lastPercentage, 30, 30, 0, 3);
            win = false;
            gameOver = true;
          } else {
            // Update display only if not exploded
            showTime(BOMB_CLOCK - timer);
            int x = calculatePercentage(timer, BOMB_CLOCK);
            if(lastPercentage != x){
              drawPercentage(16, 3, x);
              int from = percentageBarPosition(lastPercentage, 30);
              int to = percentageBarPosition(x, 30);
              if(from != to) drawProgressBar(from, to, 30, 0, 3);
              lastPercentage = x;
            }
          }
        }

        // Call buzzing unconditionally (function checks bBUZZ internally)
        buzzing();

        // Check disarm conditions only if bomb hasn't exploded
        if (timer < BOMB_CLOCK) {
            if (bp && checkPass(pass)) {
              bp = false;
              lcdClearRow(1); // Clear password input row
              printActiveGames(bp, bw, bk, 0, true);
            }

            // Pass timer by reference to checkWire/checkNFC
            if (bw && checkWire(timer, cutWires)) {
              bw = false;
              printActiveGames(bp, bw, bk, 0, true);
            }

            if (bk && checkNFC(timer)) {
              bk = false;
              printActiveGames(bp, bw, bk, 0, true);
            }

            if (!bp && !bw && !bk && pushedButton(GREEN_BTN, false, 2)) {
              win = true; // Disarmed successfully
              gameOver = true;
            }
        }
    } // end if(!gameOver) check after shock sensor
  } // end while(!gameOver)

  // Final state messages
  lcdClearRow(0); // Clear top row
  lcdClearRow(1); // Clear message row
  lcdClearRow(2); // Clear button progress row
  winMessage(win, 1);
  alarm(!win); // Trigger alarm if players lost (bomb exploded or shock)
  grenade(); // Trigger grenade effect if enabled
}