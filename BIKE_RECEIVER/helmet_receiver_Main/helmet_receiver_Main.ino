
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

//variable for nrf Setup
// Define CE and CSN pins for ESP32-S3
RF24 radio(4, 5);  // CE on GPIO 4, CSN on GPIO 5
const byte address[6] = "00001";

float dataReceived[32];

//variable for temp reading
#define DHTPIN 16
#define DHTTYPE DHT11  // DHT 11
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
float temperature = 0.0;

//object for MPU6050
MPU6050 mpu(Wire);
float speed = 0.0, inclination = 0.0;

//Variable for Ultrasonic
#define trigPin 17
#define echoPin 18
long distance_cm;

char bluetoothStatus[] = "Connected";
int relayPin = 19;
char nrfStatus[] = "Connected";


//bluetooth json data transfer
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ArduinoJson.h>

#define SERVICE_UUID "12345678-1234-1234-1234-123456789012"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-210987654321"

BLECharacteristic* pCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

unsigned long blePreviousMillis = 0;
const unsigned long bleInterval = 500;  // Send data every 500 ms


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
    // Restart advertising to allow the device to reconnect
    pServer->getAdvertising()->start();
    Serial.println("Restarting advertising...");
  }
};



void setup() {
  Serial.begin(115200);

  //Bluetooth json data trasnfer
  // Initialize BLE Device
  BLEDevice::init("ESP32_S3_Bluetooth");

  // Print the BLE device address
  String deviceAddress = BLEDevice::getAddress().toString().c_str();
  Serial.println("Device Address: " + deviceAddress);

  // Create BLE Server and set callbacks
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());  // Attach the server callbacks

  // Create BLE Service
  
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  // Start the service
  pService->start();

  // Configure and start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinInterval(100);
  pAdvertising->setMaxInterval(200);
  pAdvertising->start();

  Serial.println("BLE device is ready and advertising...");
  randomSeed(analogRead(0));  // Seed random number generator
  delay(1000);


  pinMode(relayPin, OUTPUT);
  
  loopJsonDataSend();
  //Remaining data setup
  setupNRF();
  // Wait until conditions are met
  while (true) {
    loopNRF();
    if (dataReceived[0] > 3000 && dataReceived[1] < 0.15) {
      Serial.println("Conditions met: Proceeding with execution...");
      digitalWrite(relayPin, HIGH);
      //strcpy(engineStatus, "ON");
      break;
    } else {
      Serial.println("Waiting for valid data...");
      delay(500);  // Delay to avoid spamming the serial output
    }
    Serial.println();
  }

  // Reinitialize nRF to ensure it's in a clean state
  Serial.println("Reinitializing nRF module...");
  radio.stopListening();
  delay(10);
  setupNRF();  // Call setupNRF() again to ensure proper state
  //strcpy(conn,"NRF Connected");
  setupReadTemperature();
  setupReadAngleAcceleration();
  setupReadDistance();
  loopJsonDataSend();
}


void loop() {
  //Calling the functions to read Data
  loopNRF();
  loopReadAngleAcceleration();
  loopReadTemperature();
  loopReadDistance();

  loopJsonDataSend();
}



//Set Up Files

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
  dht.temperature().getSensor(&sensor);
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




// LOOP functions

void loopNRF() {
  if (radio.available()) {

    radio.read(dataReceived, sizeof(dataReceived));
    Serial.print("Received FSR Value ");
    Serial.println(dataReceived[0]);
    Serial.print("Received Alcohol Value ");
    Serial.println(dataReceived[1]);
  } else {
    Serial.println("Radio Not Available");
  }
  if (dataReceived[0] > 3000 & dataReceived[1] < 0.15) {
    Serial.println("Helmet Worn");
  } else {
    Serial.println("Helmet Not Worn ");
  }
  //Serial.println();

  delay(10);  // Delay to reduce serial output frequency
}

void loopReadAngleAcceleration() {
  long timer = 0;
  mpu.update();

  if (millis() - timer > 1000) {  // print data every second
    //using xaceleartion for measuring the speed
    Serial.print(F("ACCELERO  X: "));
    speed = abs(mpu.getAccX());
    //Serial.println(mpu.getAccX());
    Serial.println(speed);
    //using y angle for measuring the tilting
    Serial.print(F("ANGLE     Y: "));
    inclination = abs(mpu.getAngleY());
    //Serial.println(mpu.getAngleY());
    Serial.println(inclination);
    timer = millis();
  }
}
void loopReadTemperature() {
  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    Serial.print(F("Temperature: "));
    temperature = event.temperature;
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }
  delay(10);
}
void loopReadDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  // Reading the echo pulse duration
  long duration = pulseIn(echoPin, HIGH);

  // Calculating the distance in centimeters
  distance_cm = duration * 0.034 / 2;  // Speed of sound is approximately 34,000 cm/s (divided by 2 for one-way distance)
  if (distance_cm < 400) {
    // Print the distance to the Serial monitor
    Serial.print("Distance: ");
    Serial.print(distance_cm);
    Serial.println(" cm");
  }
  Serial.println("");

  delay(10);  // Adjust delay if needed
}

void loopJsonDataSend() {

  //Bluetooth Data trasfer to Mobile App
  // Check if a device is connected
  if (deviceConnected && millis() - blePreviousMillis >= bleInterval) {
    // Create a JSON object
    StaticJsonDocument<128> doc;
    doc["HelmetConnection: "] = nrfStatus;
    doc["BluetoothStatus: "] = bluetoothStatus;
    doc["FSR Value: "] = dataReceived[0];
    doc["Alcohol Value: "] = dataReceived[1];
   
    doc["Inclination: "] = inclination;
    doc["Speed: "] = speed;
    doc["Engine Temperature: "] = temperature;
    doc["Distance: "] = distance_cm;



    // Serialize JSON to a string
    String jsonString;
    serializeJson(doc, jsonString);

    // Ensure JSON size is within BLE limits
    if (jsonString.length() > 512) {
      Serial.println("Error: JSON string exceeds BLE characteristic limit.");
      return;
    }

    // Send the JSON string via BLE
    pCharacteristic->setValue(jsonString.c_str());
    pCharacteristic->notify();  // Notify connected device

    Serial.println("Data sent: " + jsonString);
    //delay(10);  // Transmit data every 1 second
  } else {
    // If the device was previously connected but now disconnected, log it
    if (oldDeviceConnected && !deviceConnected) {
      Serial.println("Device was connected but now disconnected.");
      oldDeviceConnected = deviceConnected;
    }
  }

  // Check if the device was previously disconnected and is now connected
  if (!oldDeviceConnected && deviceConnected) {
    Serial.println("Device was disconnected but now reconnected.");
    oldDeviceConnected = deviceConnected;
  }
}
