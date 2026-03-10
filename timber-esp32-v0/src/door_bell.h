// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_DOOR_BELL_H
#define TIMBER_DOOR_BELL_H

#include <Arduino.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <BLEDevice.h>

#include "types.h"

// Forward definition.
class CameraCallbacks;

/// @brief DoorBell describes a tiny smart door bell named Tímber!
/// @details Configures the DoorBell instance hardware mappings.
class DoorBell {
    // Basic settings
    String          name_;
    String       version_;
    Preferences settings_;
    bool          online_;
    String    ip_address_;

    // Writes to ble_* properties.
    friend class CameraCallbacks;

public:
    DoorBell(const char*, const char*);
    ~DoorBell() {}

    void Setup(); // maps to sketch::setup()
    void OnLoop(); // maps to sketch::loop()
    void OnWake(); // called after Setup()

    // called for BLE events ("client notifier callback")
    static void OnData(BLERemoteCharacteristic*, uint8_t*, size_t, bool);

    void SetButton(const char*, unsigned short);

    ButtonDevice&  GetButton();
    const String&  GetName();
    const String&  GetVersion();
    bool           IsOnline();
    const String&  GetIPAddress();

protected:
    void setupCameraConnection();
    void setupWiFiConnection();
    void ensureWiFiConnected();
    void enterDeepSleep();
    void handleButtonPress();
    void requestCameraSnapshot();

    bool sendHTTPRequest(const char*);
    // bool sendLastSnapshot(const char*);

private:
    // Devices connections
    ButtonDevice   button_;
    BLEClient* ble_client_; // Camera connection
    BLERemoteCharacteristic* ble_trigger_op_; // Triggers camera capture

    // Devices states
    bool            has_button_;
    bool             btn_state_;
    bool            btn_active_;
    bool            has_sensor_;
    bool         ble_connected_;

    unsigned long         started_at_;
    unsigned long   last_activity_at_;
    unsigned long       btn_press_at_;
    unsigned long     btn_release_at_;
    unsigned long    ble_snapshot_at_;
    unsigned long    ble_lastscan_at_;

    // Camera state
    static File         snapshot_;
    static uint32_t   snap_bytes_;
    static uint32_t   rcvd_bytes_;
};

#endif