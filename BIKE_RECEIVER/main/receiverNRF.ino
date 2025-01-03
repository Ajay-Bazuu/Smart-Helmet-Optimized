/*
nRF24L01    ESP32-S3
---------    ---------
   VCC   ---->  3.3V
   GND   ---->  GND
   CE    ---->  GPIO 4
   CSN   ---->  GPIO 5
   SCK   ---->  GPIO 15
   MOSI  ---->  GPIO 7
   MISO  ---->  GPIO 6

*/
extern const byte address[6];
extern float dataReceived[32];


void loopNRF() {
  if (radio.available()) {
    //float dataReceived[32];
    radio.read(dataReceived, sizeof(dataReceived));
    Serial.print("Received FSR Value ");
    Serial.println(dataReceived[0]);
    Serial.print("Received Alcohol Value ");
    Serial.println(dataReceived[1]);

    if (dataReceived[0] > 3000 && dataReceived[1] < 0.15) {
      Serial.println("Helmet Worn");
    } else {
      Serial.println("Helmet Not Worn ");
    }
    
  } else {
    Serial.println("Not Available");
  }


  delay(150);  // Delay to reduce serial output frequency
}
