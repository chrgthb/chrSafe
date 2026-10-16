#pragma once

#include <Arduino.h>
#include <functional>

namespace chrSafe {
    using safeModeCb = std::function<void()>;

    // Set the callback
    void setSafeModeCallback(safeModeCb cb);

    // Returns true if the system is in safe mode.
    bool setup(uint8_t maxCrashes = 3, uint16_t stableMs = 10000, uint32_t magicNumber = 0xAA55AA55);
    // Returns true if the system is in safe mode.
    bool loop();
    uint16_t getStableTimeMs();
}