// this is the main receiver file that will receive two values from sender and generate different values from associated files

//header files for nrf use
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//header file for temperature reading
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

//header for MPU6050
#include "Wire.h"
#include <MPU6050_light.h>

//calling the function of other file
extern void loopNRF();
extern void loopReadTemperature();
extern void loopReadAngleAcceleration();
extern void loopReadDistance();

//variable for nrf Setup
// Define CE and CSN pins for ESP32-S3
RF24 radio(4, 5);  // CE on GPIO 4, CSN on GPIO 5
const byte address[6] = "00001";

//variable for temp reading
#define DHTPIN 16
#define DHTTYPE DHT11  // DHT 11
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

//object for MPU6050
MPU6050 mpu(Wire);

//Variable for Ultrasonic
#define trigPin 17
#define echoPin 18

void setup() {
  Serial.begin(115200);
  setupNRF();  // calling nrf for setup
  setupReadTemperature();
  setupReadAngleAcceleration();
  setupReadDistance();
}

void loop() {
  loopNRF();
  loopReadTemperature();
  loopReadAngleAcceleration();
  loopReadDistance();
}

void setupNRF() {

  // Initialize SPI communication on ESP32-S3 with specified pins
  SPI.begin(15, 6, 7, 5);  // SCK=15, MISO=6, MOSI=7, CSN=5

  radio.begin();

  // Check if the nRF24L01 module is connected
  if (radio.isChipConnected()) {
    Serial.println("nRF24L01 is connected successfully!");
  } else {
    Serial.println("ERROR: nRF24L01 is NOT connected. Please check wiring.");
    while (true)
      ;  // Stop further execution if the module is not connected
  }

  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(119);  // Example of setting a higher channel
  radio.startListening();
}
void setupReadTemperature() {
  // Initialize device.
  dht.begin();
  sensor_t sensor;
  // dht.temperature().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
}
void setupReadAngleAcceleration() {
  Wire.begin();
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while (status != 0) {}  // stop everything if could not connect to MPU6050

  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(true, true);  // gyro and accelero
  Serial.println("Done!\n");
}
void setupReadDistance() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}
