// Functions that handle reading and writing NFC cards
int blockNumber = 2;
int trailerBlock = (blockNumber / 4 * 4) + 3;

void writeNFC(byte data[]) {
  MFRC522::StatusCode status;
  byte bufferSize = 18;  // Create a variable to hold the size
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &nfcKey, &(mfrc522.uid));
  // Pass the bufferSize variable directly, not its address
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockNumber, data, bufferSize);
}

void readNFC(byte data[]) {
  MFRC522::StatusCode status;
  byte bufferSize = 18;  // Create a variable to hold the size
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &nfcKey, &(mfrc522.uid));
  // Keep passing the address for MIFARE_Read
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockNumber, data, &bufferSize);
}

void mfr_halt() {
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void waitForNewNFC() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place card on reader");
  while ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Card detected!");
  beep();
}
