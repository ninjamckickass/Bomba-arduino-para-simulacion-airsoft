// Test functions for the bomb project

void test() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("C1 or C2 or C3 or C4 or");
  lcd.setCursor(0, 1);
  lcd.print("B1 or B2 or");
  lcd.setCursor(0, 3);
  lcd.print("NFC");
  char key;
  int n = 8;
  do {
    key = keypad.getKey();
    check(3, 0, boolRead(WIRE[0]));
    check(8, 0, boolRead(WIRE[1]));
    check(13, 0, boolRead(WIRE[2]));
    check(18, 0, boolRead(WIRE[3]));
    check(3, 1, boolRead(RED_BTN));
    check(8, 1, boolRead(GREEN_BTN));
    if(key == 'A'){
      digitalWrite(ALARMPIN, HIGH);
      delay(1000);
      digitalWrite(ALARMPIN, LOW);
    }
    else if(key == 'B'){
      digitalWrite(GRENADEPIN, HIGH);
      delay(1000);
      digitalWrite(GRENADEPIN, LOW);
    }
    else if (key == '#' && n > 1) n--;
    else if (key == '*' && n <= 7) n++;
    else if (key >= '0' && key <= '9') {
      byte nums[] = {B01111110, B00110000, B01101101, B01111001, B00110011, B01011011, B01011111, B01110000, B01111111, B01111011};
      ld.write(n, nums[key - '0']);
    }
    if (accel.available()) {
      lcd.setCursor(0, 2);
      lcd.print(accel.getCalculatedX(), 2);
      lcd.print("X");
      lcd.print(accel.getCalculatedY(), 2);
      lcd.print("Y");
      lcd.print(accel.getCalculatedZ(), 2);
      lcd.print("Z");
    }
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      lcdClear(4, 3, 19, 3);
      lcd.setCursor(4, 3);

      for (byte i = 0; i < mfrc522.uid.size; i++) {
        lcd.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        lcd.print(mfrc522.uid.uidByte[i], HEX);
      }
      mfr_halt();
    }
  } while (key != 'D');
}