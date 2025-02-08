#include <math.h> // Ensure math.h is included for sin() function

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
        delay(500);  // Adjust this delay to match the speed you need
    }
}