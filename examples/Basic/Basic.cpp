#include <Arduino.h>
#include <chrSafe.h>

constexpr uint8_t maxCrashes = 3;
constexpr uint16_t stableMs = 10000;
constexpr uint32_t magicNumber = 0xAA55AA55;

uint32_t lastStatusAt = 0;
bool safeModeActive = false;

void onSafeMode() {
    // This callback runs when the crash limit has been reached.
    Serial.println("Safe mode entered: normal application startup was skipped.");
}

void setup() {
    Serial.begin(115200);

    // Register the callback before initializing the safe-mode logic.
    chrSafe::setSafeModeCallback(onSafeMode);

    // setup() returns true if the device has exceeded the allowed crash count.
    safeModeActive = chrSafe::setup(maxCrashes, stableMs, magicNumber);

    if (safeModeActive) {
        Serial.println("Device is running in safe mode.");
    } else {
        Serial.println("Normal application startup.");
    }
}

void loop() {
    // chrSafe measures stable operation time internally using millis().
    safeModeActive = chrSafe::loop();

    if (safeModeActive) {
        // Keep the device responsive while avoiding normal application behavior.
        if (millis() - lastStatusAt >= 5000) {
            lastStatusAt = millis();
            Serial.println("Safe mode is active.");
        }
        return;
    }

    // Run the normal application here.
}
