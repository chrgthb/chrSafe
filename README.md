# chrSafe

`chrSafe` is a small library for the ESP32 Arduino framework that detects repeated unsuccessful starts and enters a safe mode. It keeps a crash counter in RTC memory across software and watchdog resets, then clears the counter after the application has run for a configured stable period.

## Features

- Tracks consecutive starts using ESP32 RTC memory.
- Enters safe mode after a configurable number of unsuccessful starts.
- Calls an optional callback when safe mode is entered.
- Clears the crash counter after a configured period of stable operation.
- Clears the crash counter on a cold boot or deep-sleep wake.

## Requirements

- ESP32
- Arduino framework
- PlatformIO or another ESP32 Arduino build environment

The library uses ESP32-specific reset-reason and RTC-memory APIs and is not intended for non-ESP32 boards.

## Installation

Add this repository as a PlatformIO library dependency:

```ini
lib_deps =
    https://github.com/chrgthb/chrSafe.git
```

Or copy the library into your project's `lib` directory.

## Quick start

Register the callback before calling `setup()`. The callback is invoked during `setup()` if the crash limit has already been reached. Call `loop()` on every Arduino loop iteration; the library measures elapsed time internally using `millis()`.

```cpp
#include <Arduino.h>
#include <chrSafe.h>

void onSafeMode() {
    // Initialize or indicate the application's recovery mode here.
}

void setup() {
    Serial.begin(115200);

    chrSafe::setSafeModeCallback(onSafeMode);

    const bool safeMode = chrSafe::setup(
        3,           // Enter safe mode after 3 unsuccessful starts
        10000,       // Require 10 seconds of stable operation
        0xAA55AA55   // Identifier for this application's RTC state
    );

    if (safeMode) {
        Serial.println("Safe mode is active");
    }
}

void loop() {
    const bool safeMode = chrSafe::loop();

    if (safeMode) {
        // Run safe-mode behavior; avoid starting the normal application.
        return;
    }

    // Run normal application behavior.
}
```

## API

All functions are in the `chrSafe` namespace and declared in [`src/chrSafe.h`](src/chrSafe.h).

### `setSafeModeCallback(safeModeCb cb)`

Registers a `void()` callback that is called when `setup()` detects that the crash limit has been reached. Register it before calling `setup()`.

### `setup(uint8_t maxCrashes = 3, uint16_t stableMs = 10000, uint32_t magicNumber = 0xAA55AA55)`

Initializes the safe-mode logic for the current boot.

- `maxCrashes`: number of unsuccessful starts allowed before safe mode is entered.
- `stableMs`: time in milliseconds the application must run before the crash counter is cleared (0–65,535 ms, due to the `uint16_t` parameter).
- `magicNumber`: identifier used to recognize this library's retained RTC state.

Returns `true` when safe mode is active; otherwise returns `false`. The callback, if registered, is called before `setup()` returns `true`.

### `loop()`

Checks safe-mode state and updates the crash counter once `stableMs` has elapsed since `setup()` was called. The library uses Arduino's `millis()` internally, so no time value needs to be passed. Returns `true` while safe mode is active.

### `getStableTimeMs()`

Returns the configured stable-operation interval in milliseconds.

## License

Distributed under the MIT License. See [`LICENSE`](LICENSE).
