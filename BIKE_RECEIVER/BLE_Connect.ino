extern float FSRValue;
extern float alcoholValue;
extern float inclination;
extern float speed;
extern float distance_cm;
extern float temperature;
extern char message[80];
extern char helmetStatus[5];

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

void sendString(BLECharacteristic* characteristic, const String& value);

// BLE objects
BLEServer* pServer = NULL;
BLECharacteristic* dataChunk1Characteristic = NULL;
BLECharacteristic* dataChunk2Characteristic = NULL;
BLECharacteristic* dataChunk3Characteristic = NULL;

bool deviceConnected = false;
unsigned long previousMillisBLE = 0;
const unsigned long intervalBLE = 200;

// UUIDs for the BLE service and characteristics
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_DATACHUNK1 "beb5483e-36e1-4688-b7f5-ea07361b26a1"
#define CHAR_DATACHUNK2 "beb5483e-36e1-4688-b7f5-ea07361b26a2"
#define CHAR_DATACHUNK3 "beb5483e-36e1-4688-b7f5-ea07361b26a3"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
    BLEDevice::startAdvertising();
    Serial.println("Advertising restarted");
  }
};

void setupBLEConnect() {
  Serial.begin(115200);

  // Initialize BLE
  BLEDevice::init("Smart Helmet");
  //BLEDevice::setMTU(512);  // Ensure the client supports this MTU size

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create characteristics
  dataChunk1Characteristic = pService->createCharacteristic(CHAR_DATACHUNK1, BLECharacteristic::PROPERTY_NOTIFY);
  dataChunk2Characteristic = pService->createCharacteristic(CHAR_DATACHUNK2, BLECharacteristic::PROPERTY_NOTIFY);
  dataChunk3Characteristic = pService->createCharacteristic(CHAR_DATACHUNK3, BLECharacteristic::PROPERTY_NOTIFY);

  // Add BLE2902 descriptors to enable notifications
  dataChunk1Characteristic->addDescriptor(new BLE2902());
  dataChunk2Characteristic->addDescriptor(new BLE2902());
  dataChunk3Characteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // No preference for connection parameters
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection...");
}

void sendString(BLECharacteristic* characteristic, const String& value) {
  // Send BLE notification
  characteristic->setValue(value.c_str());
  characteristic->notify();
  Serial.println("Sent: " + value);
}

void loopBLEConnect() {
  // Generate random sensor values

  unsigned long currentMillis = millis();

  if (deviceConnected && (currentMillis - previousMillisBLE >= intervalBLE)) {
    previousMillisBLE = currentMillis;

    // Prepare and send data chunks
    String dataChunk1 = "," + String(FSRValue, 1) + "," + String(alcoholValue, 1) + "," + String(inclination, 1)  + ",";
    sendString(dataChunk1Characteristic, dataChunk1);

    String dataChunk2 = "," + String(distance_cm, 1) + "," + String(temperature, 1) + "," + String(abs(speed), 1) + ",";
    sendString(dataChunk2Characteristic, dataChunk2);

    String dataChunk3 = ","+String(message)+","+String(helmetStatus)+",";
    sendString(dataChunk3Characteristic,dataChunk3 );
  }
}
