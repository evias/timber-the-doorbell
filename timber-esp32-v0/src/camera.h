// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_CAMERA_H
#define TIMBER_CAMERA_H

#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>

#include "constants.h"
#include "door_bell.h"

class CameraCallbacks
    : public BLEAdvertisedDeviceCallbacks {

    DoorBell& timber_;

    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.getName() == "Iris-of-Timber") {
            Serial.println("[BLE] Found Iris-of-Timber! Connecting...");
            BLEDevice::getScan()->stop();

            // Connect to Iris-of-Timber
            timber_.ble_client_ = BLEDevice::createClient();
            timber_.ble_client_->connect(&advertisedDevice);

            BLERemoteService* conn = timber_.ble_client_->getService(IRISBLE_SERVICE_UUID);
            if (conn == nullptr) {
                Serial.println("[BLE] Service not found");
                return;
            }

            timber_.ble_trigger_op_ = conn->getCharacteristic(IRISBLE_TRIGGER_UUID);
            BLERemoteCharacteristic* size_op = conn->getCharacteristic(IRISBLE_SIZE_UUID);
            BLERemoteCharacteristic* data_op = conn->getCharacteristic(IRISBLE_DATA_UUID);

            if (size_op == nullptr || data_op == nullptr) {
                Serial.println("[BLE] Characteristics not found");
                return;
            }

            if (size_op->canNotify()) {
                size_op->registerForNotify(DoorBell::OnData);
            }
            if (data_op->canNotify()) {
                data_op->registerForNotify(DoorBell::OnData);
            }

            timber_.ble_connected_ = true;
            Serial.println("[BLE] Connected & ready!");
        }
    }

public:
    CameraCallbacks(DoorBell& b) : timber_(b) {}
    ~CameraCallbacks() {}
};

#endif