// - Domination
// Device acts as a control point or base station.
// Base starts in neutral state when game begins.
// Teams compete to capture and hold the base.
// When captured, base adds points to controlling team.
// If accelerometer enabled, excessive movement neutralizes the base.
void domination() {
  unsigned long reloj = 0;
  unsigned long points[2] = {0, 0};
  bool dominator[2] = {false, false};

  lcd.setCursor(0, 0);
  lcd.print(RED_TEAM);
  lcd.print("  ");
  lcd.print(GREEN_TEAM);
  lcd.setCursor(0, 1);
  lcd.print("000000    000000");



  unsigned long game_counter = millis();
  drawEmptyProgressBar(2);
  int lastPercentage = 0;
  int millisCounter = 0;
  while (reloj < RELOJ_JUEGO) {

    if (countMillis(10, game_counter)) {
      reloj++;
      millisCounter++;
      showTime(RELOJ_JUEGO - reloj);
      int x = calculatePercentage(reloj, RELOJ_JUEGO);
      if(lastPercentage != x){
        drawPercentage(16, 2, x);
        int from = percentageBarPosition(lastPercentage, 30);
        int to = percentageBarPosition(x, 30);
        if(from != to) drawProgressBar(from, to, 30, 0, 2);
        lastPercentage = x;
      }
      if (millisCounter >= 10) { //359999 max puntos
        millisCounter = 0;
        if (dominator[0]) {
          points[0]++;
          char buffer[7];
          sprintf(buffer,"%06d",points[0]);
          lcd.setCursor(0, 1);
          lcd.print(buffer);
        }
        if (dominator[1]) {
          points[1]++;
          char buffer[7];
          sprintf(buffer,"%06d",points[1]);
          lcd.setCursor(10, 1);
          lcd.print(buffer);
        }
      }
    }
    buzzing();
    if (dominator[0]) {
      if (pushedButton(GREEN_BTN, false, 3)) dominator[0] = false;
    }
    if (dominator[1]) {
      if (pushedButton(RED_BTN, false, 3)) dominator[1] = false;
    }
    if (!dominator[0] && !dominator[1]) {
      if (boolRead(GREEN_BTN) && pushedButton(RED_BTN, false, 3)) dominator[0] = true;
      if (boolRead(RED_BTN) && pushedButton(GREEN_BTN, false, 3)) dominator[1] = true;
    }
  }
  if (points[0] == points[1]) {
    lcd.setCursor(0, 3);
    lcd.print(DRAW);
  }
  else winMessage((points[1] > points[0]), 0);
  alarm(true);
}

