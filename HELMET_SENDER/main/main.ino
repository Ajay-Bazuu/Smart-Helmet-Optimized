// Declare totalValue and fsrReadValue() from readForceValue.ino
extern int totalValue;
extern void fsrReadValue();

extern float BAC, ppm;
extern void readAlcoholValue();

extern void nrfTransfer();

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Call fsrReadValue() to update totalValue
  fsrReadValue();
  // Print totalValue in main.ino
  Serial.print("Total FSR Value in main.ino: ");
  Serial.println(totalValue);

  // call readAlcoholValue() function for getting the alcohol level
  readAlcoholValue();
  Serial.print("Alcohol Level ");
  Serial.println(BAC);
  Serial.println();
  nrfTransfer(totalValue,BAC);

  delay(250);  // Wait for 250 milisecond
}
