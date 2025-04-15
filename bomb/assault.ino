// - Assault
// ALPHA team (blue) must defend the bomb during game time.
// BRAVO team (red) tries to arm the bomb and defend it until detonation.
// The bomb can be armed and disarmed multiple times until game time expires.
// Optional: Password can be required for arming/disarming.

void assault() {
  unsigned long relojBomba = 0;
  unsigned long relojJuego = 0;
  String pass = "";
  bool bp = bPASS,
       bk = bNFC;
  bool bombActive = false;
  drawEmptyProgressBar(2);
  printActiveGames(bp, false, bk, 0, true);
  lcd.setCursor(0, 1);
  lcd.print(BOMB_UNACTIVE);
  int lastPercentage = 0;
  unsigned long game_counter = millis();
  while ( relojBomba < RELOJ_BOMBA && relojJuego < RELOJ_JUEGO) {
    buzzing();
    //--------------Contador juego----------------
    if (countMillis(10, game_counter)) {
      relojJuego++;
      if (bombActive) {
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
      else showTime(RELOJ_JUEGO - relojJuego);
    }
    //-------------Chequeo protecciones activas--------------------------
    if (bp && checkPass(pass)) {
      bp = false;
      printActiveGames(bp, false, bk, 0, bombActive);
      lcdBorra(0, 1, 19, 1);
      lcd.setCursor(0, 1);
      if (bombActive) lcd.print(BOMB_ACTIVE);
      else lcd.print(BOMB_UNACTIVE);
    }
    if (bk && checkNFC(relojBomba)) {
      bk = false;
      printActiveGames(bp, false, bk, 0, bombActive);
    }
    //--------------------------------------------------------------------
    //--------------Activacion/desactivacion bomba-----------------------
    if (!bombActive && !bp && !bk && pushedButton(RED_BTN, false, 3)) {
      bombActive = true;
      bp = bPASS;
      bk = bNFC;
      printActiveGames(bp, false, bk, 0, false);
      lcdBorra(0, 1, 19, 1);
      lcdBorra(0, 3, 19, 3);
      lcd.setCursor(0, 1);
      lcd.print(BOMB_ACTIVE);
    }
    else if (bombActive && !bp && !bk && pushedButton(GREEN_BTN, false, 3)) {
      bombActive = false;
      bp = bPASS;
      bk = bNFC;
      printActiveGames(bp, false, bk, 0, true);
      lcdBorra(0, 3, 19, 3);
      lcdBorra(0, 3, 19, 3);
      lcd.setCursor(0, 1);
      lcd.print(BOMB_UNACTIVE);
    }
  }
  winMessage(!bombActive, 0);
  alarm(true);
}

