// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_SERIAL_H
#define TIMBER_SERIAL_H

#include <Arduino.h> // Serial, String

#include "constants.h"

/// @brief Prepare and send a debug message (not JSON).
/// @param message The message to send on serial port.
inline void sendDebugMessage(String message) {
    if constexpr(TIMBER_DEBUG_ENABLED) {
        Serial.printf("[DEBUG] %s\n", message.c_str());
    }
}

#endif