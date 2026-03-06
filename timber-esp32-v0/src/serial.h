// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_SERIAL_H
#define TIMBER_SERIAL_H

#include <Arduino.h>
#include <stdio.h>

#include "constants.h"

/// @brief Format elapsed milliseconds to human-readable elapsed time.
/// @param ms Milliseconds since device started.
/// @param buffer Output parameter
inline void formatElapsedTime(unsigned long ms, char* buffer) {
  unsigned long totalSeconds = ms / 1000;
  unsigned long milliseconds = ms % 1000;
  unsigned long seconds = totalSeconds % 60;
  unsigned long minutes = (totalSeconds / 60) % 60;
  unsigned long hours = (totalSeconds / 3600) % 24;
  unsigned long days = totalSeconds / 86400;

  char buf[19];
  snprintf(buffer, sizeof(buf), "%05ld %02ld:%02ld:%02ld.%03ld",
    days, hours, minutes, seconds, milliseconds);
}

/// @brief Prepare and send a debug message (not JSON).
/// @param message The message to send on serial port.
inline void sendDebugMessage(String message) {
    if constexpr(TIMBER_DEBUG_ENABLED) {
        unsigned long log_tz = millis();
        char dt[19] = ""; // 00000 00:00:00.000\0
        formatElapsedTime(log_tz, dt);

        Serial.printf("[%s] [DEBUG] %s\n", dt, message.c_str());
    }
}

#endif