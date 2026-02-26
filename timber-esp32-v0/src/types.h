// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_TYPES_H
#define TIMBER_TYPES_H

#include <Arduino.h>     // String

/// @brief  Device describes a device wired with pins to a ESP32 board.
/// @tparam NUM_PINS 
template <unsigned int NUM_PINS>
struct Device {
    String         id;
    unsigned short pins[NUM_PINS];
};

/// @brief ButtonDevice describes a press button wired with 2 pins.
struct ButtonDevice {
    Device<1> dev; // 0=TRIG
};

#endif