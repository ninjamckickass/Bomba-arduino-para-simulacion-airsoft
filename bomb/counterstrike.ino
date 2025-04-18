// - Counterstrike
// There are three ways to finish the game:
//
// 1. Game time passes and BRAVO team (red) has not succeeded in placing and arming device,
//    ALPHA team wins (blue)
// 2. BRAVO team places and arms the bomb, but ALPHA team disarms it before detonation.
//    ALPHA team (blue) wins
// 3. BRAVO team places and arms the bomb and it detonates before ALPHA team can disarm it.
//    BRAVO team (red) wins
//
// The bomb can only be armed and disarmed once.
// For game balance, bomb placement locations should be equidistant from both teams.
//
// Optional difficulty modifiers:
// - Accelerometer: Excessive movement causes premature detonation
// - Password: 5-digit code required to defuse bomb

void counterstrike() {
  unsigned long bombTimer = 0;
  unsigned long gameTimer = 0;
  String pass = "";
  bool bp = bPASS,
       bk = bNFC;
  bool win = false;
  bool bombActive = false;

  printActiveGames(bp, false, bk, 0, false);
  int lastPercentage = 0;
  unsigned long game_counter = millis();
  while (gameTimer < GAME_CLOCK && !bombActive) {
    if (countMillis(10, game_counter)) {
      gameTimer++;
      showTime(GAME_CLOCK - gameTimer);
    }
    if (bp && checkPass(pass)) {
      bp = false;
      printActiveGames(bp, false, bk, 0, false);
    }
    if (bk && checkNFC(bombTimer)) {
      bk = false;
      printActiveGames(bp, false, bk, 0, false);
    }
    if (!bp && !bk && pushedButton(RED_BTN, false, 3)) bombActive = true;
    buzzing();
  }
  if (gameTimer < GAME_CLOCK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(BOMB_ACTIVE);
    delay(1000);
    lcd.clear();
    bp = bPASS;
    bk = bNFC;
    drawEmptyProgressBar(2);
    printActiveGames(bp, false, bk, 0, true);
    game_counter = millis();
    while (bombTimer < BOMB_CLOCK && !win) {
      if (countMillis(10, game_counter)) {
        bombTimer++;
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
      buzzing();
      if (bombTimer >= BOMB_CLOCK) {
        bombTimer = BOMB_CLOCK;
        drawProgressBar(lastPercentage, 30, 30, 0, 2);
      }
      if (bp && checkPass(pass)) {
        bp = false;
        lcdClear(0, 1, 19, 1);
        printActiveGames(bp, false, bk, 0, true);
      }
      if (bk && checkNFC(bombTimer)) {
        bk = false;
        printActiveGames(bp, false, bk, 0, true);
      }
      if (!bp && !bk && pushedButton(GREEN_BTN, false, 3)) {
        win = true;
      }
    }
  }
  winMessage(win, 1);
  alarm(true);
}
