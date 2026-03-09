// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_IRIS_TESTER_BLUETOOTH_H
#define TIMBER_IRIS_TESTER_BLUETOOTH_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

class TestServerCallbacks
    : public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer)
    {
        Serial.println("BLE Client connected");
    }

    void onDisconnect(BLEServer* pServer)
    {
        Serial.println("BLE Client disconnected");
        BLEDevice::startAdvertising();
    }
};

class TestCharacteristicsCallbacks
    : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *op)
    {
        String value = op->getValue();

        // We should only reactor to TRIGGER operation.
        if (value.length() == 0 || value[0] != 1) {
            return ;
        }

        Serial.println("Capture triggered by client");
    }
};

#endif