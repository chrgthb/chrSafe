#include "chrSafe.h"
#include <esp_system.h>

namespace chrSafe {
    // RTC memory variables: they survive software and watchdog resets.
    // RTC_NOINIT_ATTR ensures the bootloader does not overwrite them at startup.
    RTC_NOINIT_ATTR uint32_t _rtcMagicMarker;
    RTC_NOINIT_ATTR uint32_t _crashCounter;

    // Settings
    uint8_t _maxCrashes;       // Number of failed starts before entering Safe Mode
    uint16_t _stableMs;         // The code is stable after this much time without errors
    uint32_t _magicNumber;     // Used to check whether a cold boot occurred
    uint32_t _startedAt;

    bool _isSafeMode = false;
    bool _systemStable = false;

    safeModeCb _safeModeCb = nullptr;

    // --- Internal helper function for events ---
    void setSafeModeCallback(safeModeCb cb) { _safeModeCb = cb; }

    bool setup(uint8_t maxCrashes, uint16_t stableMs, uint32_t magicNumber) {
        _maxCrashes = maxCrashes;
        _stableMs = stableMs;
        _magicNumber = magicNumber;
        _startedAt = millis();
        _systemStable = false;

        // Get the reset reason
        esp_reset_reason_t reason = esp_reset_reason();

        // 1. Check for a cold boot (power-on)
        // If the marker does not match our magic number, this is the first start
        if (_rtcMagicMarker != _magicNumber) {
            // Cold boot: initialize the crash counter
            _rtcMagicMarker = _magicNumber;
            _crashCounter = 0;
        } else if (reason == ESP_RST_DEEPSLEEP) {
            _crashCounter = 0;
        }

        // 2. Check for Safe Mode
        if (_crashCounter >= _maxCrashes) {
            _isSafeMode = true;
            if (_safeModeCb) {
                _safeModeCb();
            }
            return true;
        }

        // 3. Increment the crash counter on a normal start.
        // It is reset in loop() only after the system has run stably for the specified time.
        _crashCounter++;

        return false;
    }

    bool loop() {
        if (_isSafeMode) return true;

        // --- CHECK FOR NORMAL OPERATION ---
        // If STABLE_TIME_MS has elapsed without a crash, reset the counter
        if (!_systemStable && millis() - _startedAt >= _stableMs) {
            _crashCounter = 0; 
            _systemStable = true;
        }

        return false;
    }

    uint16_t getStableTimeMs() {
        return _stableMs;
    }

    void rebootToSafeMode() {
        // Make the next setup() see the crash limit as reached.
        _rtcMagicMarker = _magicNumber;
        _crashCounter = _maxCrashes;
        esp_restart();
    }

}