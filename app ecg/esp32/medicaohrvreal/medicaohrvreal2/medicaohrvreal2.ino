#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define PERIPHERAL_NAME "Souvik's IoT Device"
#define SERVICE_UUID "CD9CFC21-0ECC-42E5-BF22-48AA715CA112"
#define CHARACTERISTIC_UUID "142F29DD-B1F0-4FA8-8E55-5A2D5F3E2471"

// Define the pin used for peak detection. Change this if needed.
#define PEAK_PIN A0

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Volatile variables used within the ISR and main loop
volatile unsigned long lastPeakTime = 0;   // Timestamp of the last detected peak
volatile unsigned long currentRR = 0;        // Current RR interval in ms
volatile unsigned long previousRR = 0;       // Previous RR interval in ms
volatile float hrvValue = 0;                 // Computed HRV value (ms difference)
volatile bool newHRV = false;                // Flag indicating a new HRV value is ready

// Interrupt Service Routine (ISR) triggered on the rising edge of the ECG pulse.
void IRAM_ATTR onPeakDetected() {
  unsigned long currentTime = millis();
  if (lastPeakTime != 0) {
    currentRR = currentTime - lastPeakTime;
    if (previousRR != 0) {
      // Compute HRV as the absolute difference between successive RR intervals.
      if (currentRR > previousRR)
        hrvValue = currentRR - previousRR;
      else
        hrvValue = previousRR - currentRR;
      newHRV = true;  // Indicate a new HRV value is available.
    }
    previousRR = currentRR;
  }
  lastPeakTime = currentTime;
}

// BLE Server callback to track connection events.
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

  // Configure the peak detection pin and attach an interrupt on rising edge.
  pinMode(PEAK_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PEAK_PIN), onPeakDetected, RISING);

  // Initialize BLE
  BLEDevice::init(PERIPHERAL_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_NOTIFY
  );

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
}

void loop() {
  // If a BLE client is connected and a new HRV value is ready, send it.
  if (deviceConnected && newHRV) {
    // Safely read the volatile HRV value.
    noInterrupts();
    float currentHRV = hrvValue;
    newHRV = false;
    interrupts();

    Serial.print("HRV value (ms): ");
    Serial.println(currentHRV);

    // Send the HRV value as a BLE notification (4 bytes, as a float).
    pCharacteristic->setValue((uint8_t*)&currentHRV, sizeof(currentHRV));
    pCharacteristic->notify();
  }
  delay(10);  // Small delay to allow other tasks to run.
}
