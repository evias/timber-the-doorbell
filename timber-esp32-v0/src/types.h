// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_TYPES_H
#define TIMBER_TYPES_H

#include <Arduino.h>
#include <Wire.h>

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

/// @brief GestureDevice describes a 3D gesture sensor wired with 3 (data) pins.
struct GestureDevice {
    Device<3>  dev; // 0=INT,1=SDA,2=SCL
    uint8_t   addr; // PAJ7620_CHIP_ADDR

    void SelectBank(uint8_t bank) {
        Write(0xEF, bank);
        delay(50);  // Wait for bank switch
    }

    /// @brief Uses the Wire protocol to address the sensor and set a val for reg.
    /// @param reg A registry address, e.g. 0xEF for bank selection registry.
    /// @param val A value, e.g. 0x00 for 0 or 0x01 for 1.
    void Write(uint8_t reg, uint8_t val) const {
        Wire.beginTransmission(addr);
        Wire.write(reg);
        Wire.write(val);
        int error = Wire.endTransmission(true);

        if (error != 0) {
            Serial.printf("[ERROR] Write 0x%02X failed: %d\n", reg, error);
        }
        delay(10);  // Small delay between operations
    }

    /// @brief Uses the Wire protocol to read values from the sensor for reg.
    /// @param reg A registry address, e.g. 0xEF for bank selection registry.
    /// @return A value for the registry address reg.
    uint8_t Read(uint8_t reg) const {
        Wire.beginTransmission(addr);
        Wire.write(reg);
        Wire.endTransmission(false);  // Repeated START
        delay(5);

        Wire.requestFrom(addr, 1, true);
        delay(5);

        uint8_t value = Wire.read();
        return value;
    }
};

#endif