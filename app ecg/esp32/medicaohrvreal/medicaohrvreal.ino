#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Service name to be broadcasted to the outside world
#define PERIPHERAL_NAME "Souvik's IoT Device"
#define SERVICE_UUID "CD9CFC21-0ECC-42E5-BF22-48AA715CA112"
#define CHARACTERISTIC_UUID "142F29DD-B1F0-4FA8-8E55-5A2D5F3E2471"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Class defines methods called when a device connects and disconnects from the service
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("BLE Client Connected");
  }

  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("BLE Client Disconnected");
    BLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);
  // Initialize BLE
  BLEDevice::init(PERIPHERAL_NAME);

  // Create the server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create the service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create the characteristic for data transfer (notify property)
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_NOTIFY
  );

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
}

void loop() {
  if (deviceConnected) {
    // Read the analog value from pin A0
    int analogValue = analogRead(A0);

    // Convert the raw reading to a voltage value.
    // For a 12-bit ADC with a 3.3V reference, the max reading is 4095.
    float voltage = analogValue * (3.3 / 4095.0);

    // Print the raw analog reading and the computed voltage to the Serial Monitor.
    Serial.print("Analog reading: ");
    Serial.print(analogValue);
    Serial.print(" -> Voltage: ");
    Serial.print(voltage, 3);  // Print with 3 decimal places.
    Serial.println(" V");

    // Send the voltage value via BLE notification.
    // We send the float (4 bytes) as raw bytes.
    pCharacteristic->setValue((uint8_t*)&voltage, sizeof(voltage));
    pCharacteristic->notify();

    // Delay between readings (adjust as needed)
    delay(500);
  }
}
