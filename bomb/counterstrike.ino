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
// - Sling mounting points available for transport (sling not included)

void counterstrike() {
  unsigned long relojBomba = 0;
  unsigned long relojJuego = 0;
  String pass = "";
  bool bp = bPASS,
       bk = bNFC;
  bool win = false;
  bool bombActive = false;

  printActiveGames(bp, false, bk, 0, false);
  int lastPercentage = 0;
  unsigned long game_counter = millis();
  while (relojJuego < RELOJ_JUEGO && !bombActive) {
    if (countMillis(10, game_counter)) {
      relojJuego++;
      showTime(RELOJ_JUEGO - relojJuego);
    }
    if (bp && checkPass(pass)) {
      bp = false;
      printActiveGames(bp, false, bk, 0, false);
    }
    if (bk && checkNFC(relojBomba)) {
      bk = false;
      printActiveGames(bp, false, bk, 0, false);
    }
    if (!bp && !bk && pushedButton(RED_BTN, false, 3)) bombActive = true;
    buzzing();
  }
  if (relojJuego < RELOJ_JUEGO) {
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
    while (relojBomba < RELOJ_BOMBA && !win) {
      if (countMillis(10, game_counter)) {
        relojBomba++;
        showTime(RELOJ_BOMBA - relojBomba);
        int x = calculatePercentage(relojBomba, RELOJ_BOMBA);
        if (lastPercentage != x) {
          drawPercentage(16, 2, x);
          int from = percentageBarPosition(lastPercentage, 30);
          int to = percentageBarPosition(x, 30);
          if (from != to) drawProgressBar(from, to, 30, 0, 2);
          lastPercentage = x;
        }
      }
      buzzing();
      if (relojBomba >= RELOJ_BOMBA) {
        relojBomba = RELOJ_BOMBA;
        drawProgressBar(lastPercentage, 30, 30, 0, 2);
      }
      if (bp && checkPass(pass)) {
        bp = false;
        lcdBorra(0, 1, 19, 1);
        printActiveGames(bp, false, bk, 0, true);
      }
      if (bk && checkNFC(relojBomba)) {
        bk = false;
        printActiveGames(bp, false, bk, 0, true);
      }
      if (!bp && !bk && pushedButton(GREEN_BTN, false, 3)) { // && green button pulsado
        win = true;
      }
    }
  }
  winMessage(win, 1);
  alarm(true);
}

