// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_SNAPS_CAMERA_H
#define TIMBER_SNAPS_CAMERA_H

#include <Arduino.h> // String
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h> // BLEServer, BLECharacteristic
#include <BLEUtils.h>

#include "esp_camera.h" // camera_fb_t
#include "types.h"

// Forward definitions.
class IrisServerCallbacks;
class IrisCharacteristicsCallbacks;

/// @brief Camera describes a tiny smart door bell named Tímber!
/// @details Configures the Camera instance hardware mappings.
class Camera {
    friend class IrisServerCallbacks; // Writes to online_
    friend class IrisCharacteristicsCallbacks; // Writes to current_frame_

    String          name_;
    String       version_;

    BLECharacteristic*   trigger_op_;
    BLECharacteristic*      size_op_;
    BLECharacteristic*      data_op_;
    BLEServer*           ble_server_;
    BLEService*         ble_service_;

    bool          online_;
    camera_fb_t*  current_frame_;
public:
    Camera(const char*, const char*);
    ~Camera() {}

    void Setup(); // maps to sketch::setup()
    void OnLoop(); // maps to sketch::loop()

    const String&  GetName();
    const String&  GetVersion();

protected:
    bool setupCameraDevice();
    void setupBluetoothServer();
    void setupBluetoothAdvert();
};

#endif