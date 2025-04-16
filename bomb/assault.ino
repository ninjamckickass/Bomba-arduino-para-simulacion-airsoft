// - Assault
// ALPHA team (blue) must defend the bomb during game time.
// BRAVO team (red) tries to arm the bomb and defend it until detonation.
// The bomb can be armed and disarmed multiple times until game time expires.
// Optional: Password can be required for arming/disarming.

void assault() {
  unsigned long bombTimer = 0;
  unsigned long gameTimer = 0;
  String pass = "";
  bool bp = bPASS,
       bk = bNFC;
  bool bombActive = false;
  drawEmptyProgressBar(2);
  printActiveGames(bp, false, bk, 0, true); // Initially, Green defends (needs Green button)
  lcd.setCursor(0, 1);
  lcd.print(BOMB_UNACTIVE);
  int lastPercentage = 0;
  unsigned long game_counter = millis();
  // Use global GAME_CLOCK and BOMB_CLOCK
  while (bombTimer < BOMB_CLOCK && gameTimer < GAME_CLOCK) {
    buzzing();
    //--------------Game counter----------------
    if (countMillis(10, game_counter)) {
      gameTimer++;
      if (bombActive) {
        bombTimer++;
        // Check if bomb detonated BEFORE updating display
        if (bombTimer >= BOMB_CLOCK) {
            bombTimer = BOMB_CLOCK; // Cap timer
            showTime(0); // Show 00:00:00:00
            drawPercentage(16, 2, 100);
            drawProgressBar(lastPercentage, 30, 30, 0, 2);
            // Don't break here, let loop condition handle exit
        } else {
            showTime(BOMB_CLOCK - bombTimer);
            int x = calculatePercentage(bombTimer, BOMB_CLOCK);
            if (lastPercentage != x) {
              drawPercentage(16, 2, x);
              int from = percentageBarPosition(lastPercentage, 30);
              int to = percentageBarPosition(x, 30);
              if (from != to) drawProgressBar(from, to, 30, 0, 2);
              lastPercentage = x;
            }
        }
      }
      else {
          // Show game time remaining if bomb not active
          showTime(GAME_CLOCK - gameTimer);
          // Clear bomb progress bar if it was previously active
          if (lastPercentage != 0) {
              drawEmptyProgressBar(2);
              lastPercentage = 0;
          }
      }
    }

    // Only check activation/deactivation if bomb hasn't detonated and game hasn't ended
    if (bombTimer < BOMB_CLOCK && gameTimer < GAME_CLOCK) {
        //-------------Check active protections--------------------------
        // Pass bombTimer by reference to checkNFC
        if (bp && checkPass(pass)) {
          bp = false;
          // Update prompt based on current bomb state (true=Green, false=Red)
          printActiveGames(bp, false, bk, 0, !bombActive);
          lcdClearRow(1); // Clear password input row
          lcd.setCursor(0, 1);
          if (bombActive) lcd.print(BOMB_ACTIVE);
          else lcd.print(BOMB_UNACTIVE);
        }
        if (bk && checkNFC(bombTimer)) {
          bk = false;
          // Update prompt based on current bomb state
          printActiveGames(bp, false, bk, 0, !bombActive);
        }
        //--------------------------------------------------------------------
        //--------------Bomb activation/deactivation-----------------------
        // Activate (Red Button)
        if (!bombActive && !bp && !bk && pushedButton(RED_BTN, false, 3)) {
          bombActive = true;
          bombTimer = 0; // Reset bomb timer on activation
          lastPercentage = 0; // Reset progress bar percentage
          bp = bPASS; // Reset protections for deactivation
          bk = bNFC;
          printActiveGames(bp, false, bk, 0, true); // Now Green needs to deactivate
          lcdClearRow(1); // Clear status row
          lcdClearRow(3); // Clear button progress row
          lcd.setCursor(0, 1);
          lcd.print(BOMB_ACTIVE);
          drawEmptyProgressBar(2); // Draw empty progress bar for bomb timer
        }
        // Deactivate (Green Button)
        else if (bombActive && !bp && !bk && pushedButton(GREEN_BTN, false, 3)) {
          bombActive = false;
          bp = bPASS; // Reset protections for activation
          bk = bNFC;
          printActiveGames(bp, false, bk, 0, false); // Now Red needs to activate
          lcdClearRow(1); // Clear status row
          lcdClearRow(3); // Clear button progress row
          lcd.setCursor(0, 1);
          lcd.print(BOMB_UNACTIVE);
          // Clear bomb progress bar display
          drawEmptyProgressBar(2);
          lastPercentage = 0;
        }
    } // end check activation/deactivation

  } // end while

  // Game Over
  lcdClearRow(0); // Clear top row
  lcdClearRow(1); // Clear status row
  lcdClearRow(3); // Clear button progress row

  // Determine winner: Green wins if game time ran out OR bomb was inactive when bomb timer ran out. Red wins if bomb timer ran out while active.
  bool greenWins = (gameTimer >= GAME_CLOCK) || !bombActive;
  winMessage(greenWins, 0); // Show winner on top row
  alarm(true); // Sound alarm at end of game
}

