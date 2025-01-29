extern void setupBLEConnect();
extern void loopBLEConnect();

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
long timer = 0;
//variable for nrf Setup

char helmetStatus[5] = "";

// Define CE and CSN pins for ESP32-S3
RF24 radio(4, 5);  // CE on GPIO 4, CSN on GPIO 5
const byte address[6] = "00001";

float dataReceived[2];------------------------------------ 7
float FSRValue = 00, alcoholValue = 0;
//variable for temp reading
#define DHTPIN 16
#define DHTTYPE DHT11  // DHT 11
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
float temperature = 0.0;

//object for MPU6050
MPU6050 mpu(Wire);
float speed = 0.0;
float inclination = 0.0;

//Variable for Ultrasonic
#define trigPin 17
#define echoPin 18
float distance_cm = 0;


int relayPin = 21;
char message[80];
int const buzzerPin = 45;  //Buzzer Pin


void setup() {
  Serial.begin(115200);
  setupBLEConnect();
  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);

  setupNRF();
  // Wait until conditions are met
  while (true) {
    loopNRF();
    if (dataReceived[0] > 1500 && dataReceived[1] < 0.15) {
      Serial.println("Conditions met: Proceeding with execution...");
      digitalWrite(relayPin, HIGH);

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
}


void loop() {
  //Calling the functions to read Data

  //loopNRF();
  loopReadAngleAcceleration();
  loopReadTemperature();
  loopReadDistance();
  messageDisplay();
  Serial.println(message);
  loopBLEConnect();
  //delay(500);
  /* FSRValue
  alcoholValue
  speed
  inclination
  temperature
  distance_cm
  message
*/
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
    FSRValue = dataReceived[0];
    Serial.println(FSRValue);
    Serial.print("Received Alcohol Value ");
    alcoholValue = dataReceived[1];
    Serial.println(alcoholValue);
  } else {
    Serial.println("Radio Not Available");
    FSRValue = 0;
    alcoholValue = 0;
  }
  if (dataReceived[0] > 1500 & dataReceived[1] < 0.15) {
    Serial.println("Helmet Worn");
  } else {
    Serial.println("Helmet Not Worn ");
  }
  //Serial.println();

  delay(150);  // Delay to reduce serial output frequency
}

// void loopReadAngleAcceleration() {
//   mpu.update();

//   if (millis() - timer > 2) {  // print data every 2econd
//     //using xaceleartion for measuring the speed
//     Serial.print(F("ACCELERO  X: "));
//     speed = abs(mpu.getAccX())*9.8; //changing the unit into m/s2
//     //Serial.println(mpu.getAccX());
//     Serial.println(speed);
//     //using y angle for measuring the tilting
//     Serial.print(F("ANGLE     Y: "));
//     inclination = abs(mpu.getAngleY());
//     //Serial.println(mpu.getAngleY());
//     Serial.println(inclination);
//     timer = millis();
//   }
// }
// void loopReadAngleAcceleration() {
//   mpu.update();

//   if (millis() - timer > 100) {  // Print data every 100ms (0.1 seconds)
//     float deltaTime = (millis() - timer) / 1000.0;  // Convert milliseconds to seconds

//     // Using x acceleration for measuring speed
//     float xAccel = mpu.getAccX() * 9.8;  // Convert acceleration to m/s^2
//     if (abs(xAccel) > 0.2) {  // Ignore small noise values
//       speed += abs(xAccel) * deltaTime;  // Integrate acceleration over time
//     }

//     // Print speed
//     Serial.print(F("Speed (m/s): "));
//     Serial.println(speed);

//     // Using Y angle for measuring tilt (inclination)
//     inclination = abs(mpu.getAngleY());

//     // Print tilt angle
//     Serial.print(F("Inclination (degrees): "));
//     Serial.println(inclination);

//     timer = millis();
//   }
// } // this function works well. just adding little retardation in below function
// void loopReadAngleAcceleration() {
//     mpu.update();

//     if (millis() - timer > 2) {  // Print data every 100ms (0.1 seconds)
//         float deltaTime = (millis() - timer) / 1000.0;  // Convert milliseconds to seconds

//         // Using x acceleration for measuring speed
//         float xAccel = mpu.getAccX() * 9.8;  // Convert acceleration to m/s²
//         speed += xAccel * deltaTime;  // Integrate signed acceleration

//         // Apply friction to reduce speed when there's no significant acceleration
//         if (abs(xAccel) <= 0.2) {  // Negligible acceleration
//             speed *= 0.9;  // Apply friction
//             if (abs(speed) < 0.1) speed = 0.0;  // Stop speed when very small
//         }
//         //speed=abs(speed);

//         // Print absolute speed
//         Serial.print(F("Speed (m/s): "));
//         Serial.println(abs(speed));

//         // Using Y angle for measuring tilt (inclination)
//         inclination = abs(mpu.getAngleY());

//         // Print tilt angle
//         Serial.print(F("Inclination (degrees): "));
//         Serial.println(inclination);

//         timer = millis();  // Update the timer
//     }
// }

void loopReadAngleAcceleration() {
    mpu.update();

    // Check if enough time has passed to perform the calculations
    if (millis() - timer > 2) {  // Perform calculations every 2ms
        // Calculate the time difference (deltaTime) in seconds
        float deltaTime = (millis() - timer) / 1000.0;

        // Get acceleration in the X direction and convert to m/s²
        float xAccel = mpu.getAccX() * 9.8;

        // Integrate acceleration to compute speed
        // if (xAccel>0.2){
        //          speed += xAccel * deltaTime;
        // }else{
        //   speed=0;
        // }
 

        // Apply friction to reduce speed when acceleration is negligible
        if (abs(xAccel) <= 0.2) {  // Negligible acceleration threshold
            speed *= 0.9;  // Apply friction
            if (abs(speed) < 2) speed = 0.0;  // Stop speed when very small
        }

        // Print the absolute value of speed
        Serial.print(F("Speed (m/s): "));
        Serial.println(abs(speed));

        // Get and print the inclination (absolute Y angle)
        inclination = abs(mpu.getAngleY());
        Serial.print(F("Inclination (degrees): "));
        Serial.println(inclination);

        // Update the timer for the next loop
        timer = millis();
    }
}



void loopReadTemperature() {
  // Delay between measurements.
  delay(350);
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
  //delay(10);
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
void messageDisplay() {
  bool warningActive = false;

  if (FSRValue < 1500) {
    strcpy(message, "Helmet Not Worn");
    warningActive = true;
  } else if (alcoholValue > 0.15) {
    strcpy(message, "Alcohol Detected!!!");
    warningActive = true;
  } else if (distance_cm < 20) {
    strcpy(message, "Watch Out Back!!!");
    warningActive = true;
  } else if (inclination > 20) {
    strcpy(message, "Max Inclined!!!");
    warningActive = true;
  } else if (speed > 60) {
    strcpy(message, "Max Speed!!!");
    warningActive = true;
  } else if (temperature > 40) {
    strcpy(message, "Engine Overheated!!!");
    warningActive = true;
  }
  // else if(!radio.available){
  //   strcpy(message, "Helmet Unpaired");
  // }
  else {
    strcpy(message, "Everything is Okay");
  }

  if (radio.available()){
    strcpy(helmetStatus, "1");
  }else{
   strcpy(helmetStatus, "0");
  }

  if (warningActive) {
    digitalWrite(buzzerPin, HIGH);
    delay(50);
    digitalWrite(buzzerPin, LOW);
    delay(50);

  } else {
    digitalWrite(buzzerPin, LOW);
  }
}
