// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_DOOR_BELL_H
#define TIMBER_DOOR_BELL_H

#include <Arduino.h> // String
#include <Preferences.h>
#include "types.h"

/// @brief DoorBell describes a tiny smart door bell named Tímber!
/// @details Configures the DoorBell instance hardware mappings.
class DoorBell {
    bool          online_;
    String    ip_address_;

    String          name_;
    String       version_;
    bool      has_button_;
    Preferences settings_;

    ButtonDevice  button_;
    bool       btn_state_;
    bool      btn_active_;

    unsigned long btn_press_tz_;
    unsigned long btn_release_tz_;

public:
    DoorBell(const char*, const char*);
    ~DoorBell() {}

    void Setup(); // maps to sketch::setup()
    void OnLoop(); // maps to sketch::loop()
    void OnWake(); // called after Setup()

    void SetButton(const char*, unsigned short);

    ButtonDevice& GetButton();
    const String& GetName();
    const String& GetVersion();
    bool          IsOnline();
    const String& GetIPAddress();

protected:
    void setupWiFiConnection();
    void ensureWiFiConnected();
    void handleButtonPress();
    void enterDeepSleep();

    bool sendHTTPRequest(const char*);
};

#endif