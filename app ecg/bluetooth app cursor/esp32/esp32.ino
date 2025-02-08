#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Service name to be broadcasted to the outside world
#define PERIPHERAL_NAME "Souvik's IoT Device"
#define SERVICE_UUID "CD9CFC21-0ECC-42E5-BF22-48AA715CA112"
#define CHARACTERISTIC_UUID "142F29DD-B1F0-4FA8-8E55-5A2D5F3E2471"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t value = 0;

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

    // Initialize BLE
    BLEDevice::init(PERIPHERAL_NAME);

    // Create the server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create the service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create the characteristic for data transfer
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
        // Generate a dummy HRV signal using a sine wave
        static float time = 0; // Static variable to keep track of time
        value = (uint8_t)(127.5 * (sin(time) + 1)); // Scale sine wave to 0-255
        time += 0.1; // Increment time for the next value
        
        // Send the value via BLE notification
        pCharacteristic->setValue(&value, 1); // Sending one byte (the first byte of value)
        pCharacteristic->notify();
        
        // Simulate a delay between each data point
        delay(5000);  // Adjust this delay to match the speed you need
    }
}
