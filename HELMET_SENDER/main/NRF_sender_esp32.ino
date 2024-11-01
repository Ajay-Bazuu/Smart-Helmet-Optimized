/*-Pin Mapping
100uf capacitor across vcc and gnd
nRF24L01	ESP32
VCC	3.3V
GND	GND
CE	GPIO 26
CSN (CS)	GPIO 27
SCK	GPIO 14
MOSI	GPIO 12
MISO	GPIO 25
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Define CE and CSN pins for ESP32
RF24 radio(26, 27);  // CE on GPIO 26, CSN on GPIO 27

const byte address[6] = "00001";
int l = 1;

void nrfTransfer(float totalValue, float BAC) {
  //setup starts done for limiting the execution for only once when the loop of main.ino call this function repeatedly
  if (l == 1) {
    // Initialize SPI communication for ESP32 with specified pins
    SPI.begin(14, 25, 12, 27);  // SCK=14, MISO=25, MOSI=12, CSN=27

    radio.begin();

    // Check if the nRF24L01 module is connected
    if (radio.isChipConnected()) {
      Serial.println("nRF24L01 is connected successfully!");
    } else {
      Serial.println("ERROR: nRF24L01 is NOT connected. Please check wiring.");
      while (true)
        ;  // Stop further execution if the module is not connected
    }

    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MAX);
    radio.setChannel(119);
    radio.setDataRate(RF24_2MBPS);
    radio.stopListening();
    l++;
    delay(1000);
  }
  //set up ends

  const float dataToSend[2] = { totalValue, BAC };
  radio.write(dataToSend, sizeof(dataToSend));
  delay(100);
}
