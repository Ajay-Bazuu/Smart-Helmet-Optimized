int const mq3Pin = 35;        // pin for reading value of alcohol from MQ3 sensor
int measureAlcoholValue = 0;  // variable to store alcohol value
float initialAlcoholValue = 0;
float finalAlcoholValue = 0;

//Values required for Conversion
const float R_load = 100.0;  // 100-ohm load resistor used in circuit
float R0 = 10000.0;          // Assume R0 is pre-calibrated in clean air
const float m = -0.45;       // Slope from the datasheet for ethanol
const float b = 0.9;         // Intercept from the datasheet for ethanol
float BAC = 0;
int j = 1;

void readAlcoholValue() {
  if ( j == 1) {
    // put your setup code here, to run once:
    pinMode(mq3Pin, INPUT);
    Serial.begin(115200);
    delay(5000);
    for (int i = 1; i <= 10; i++) {
      initialAlcoholValue = initialAlcoholValue + analogRead(mq3Pin);
      delay(500);
    }
    initialAlcoholValue = initialAlcoholValue / 10;
    delay(500);
    j++;
  }
  //Setup Ends

  measureAlcoholValue = analogRead(mq3Pin);

  // Serial.print("Average Value: ");
  // Serial.println(initialAlcoholValue);

  // Serial.print("ALcohol Value: ");
  // Serial.println(measureAlcoholValue);

  if (measureAlcoholValue < initialAlcoholValue) {
    finalAlcoholValue = 0;
  } else {
    finalAlcoholValue = measureAlcoholValue - initialAlcoholValue;
  }
  // Serial.print("Final Value: ");
  // Serial.println(finalAlcoholValue);

  //changing raw values to ppm and BAC
  float voltage = (finalAlcoholValue / 4095) * 3.3;
  //to calculate ppm voltage is required
  // 5: 5v power provided, 4095 max output in 12 bit resolution
  float Rs = ((3.3 - voltage) / voltage) * R_load;  // Rs using 5V power supply
  float ratio = Rs / R0;                            // Ratio of Rs to R0

  // Calculate PPM (ethanol concentration)
  float ppm = pow(10, ((log10(ratio) - b) / m));
  // Serial.print("Parts per Million: ");
  // Serial.println(ppm);
  //Conversion in BAC
  BAC = ppm * 0.000048;
  // Serial.print("Blood ALcohol COncentration: ");
  // Serial.println(BAC);
  // Serial.println();

  delay(50);
}

