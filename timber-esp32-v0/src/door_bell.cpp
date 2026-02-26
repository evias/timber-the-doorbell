// SPDX-License-Identifier: BSD-3-Clause
#include <Arduino.h>    // Serial, String, digitalWrite, pinMode
#include <WiFi.h>       // WiFi
#include <HTTPClient.h> // HTTPClient
#include <esp_sleep.h>  // esp_sleep_x

#include "door_bell.h"
#include "constants.h"
#include "serial.h"

/// @brief DoorBell public constructor, says Hi.
/// @param name A friendly name for your DoorBell, e.g. "Tímber".
/// @param version A version number, recommended semantic versioning, e.g. "1.0.0".
DoorBell::DoorBell(const char* name, const char* version)
    : name_(String(name)),
      version_(String(version)),
      online_(false),
      ip_address_("Unknown"),
      has_button_(false),
      btn_state_(HIGH), // HIGH=not pressed
      btn_active_(false)
{
    sendDebugMessage(String("Hola! I am Timber :]"));
}

/// @brief The Setup method initializes configured pin numbers.
/// @details This method is called from the sketch's setup() function.
void DoorBell::Setup()
{
    // Configure ultrasonic sensor pins
    if (has_button_) {
        pinMode(button_.dev.pins[1], INPUT_PULLUP);
    }

    setupWiFiConnection();

    sendDebugMessage(String("DoorBell setup completed"));
}

/// @brief The OnWake method identifies the wakeup cause, if it's boot do nothing.
/// @details This method is called from the sketch's setup() function and makes
///          sure that given a wakeup-by-button, the button press is also handled.
void DoorBell::OnWake() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            handleButtonPress();

            // Go back to sleep after handling button press
            delay(500);
            enterDeepSleep();
            return;
        default:
            break;
    }
}

/// @brief The OnLoop method verifies for button press actions.
/// @details This method is called from the sketch's loop() function.
void DoorBell::OnLoop() {
    bool  old_ = btn_state_;
    btn_state_ = digitalRead(PRESS_BUTTON_PIN);

    if (old_ == HIGH && btn_state_ == LOW) {
        btn_press_tz_ = millis();
        btn_active_ = true;
        Serial.println("[v] Button pressed...");
    }
    else if (old_ == LOW && btn_state_ == HIGH) {
        btn_release_tz_ = millis();
        btn_active_ = false;

        unsigned long pressDuration = btn_release_tz_ - btn_press_tz_;
        if (pressDuration >= DEBOUNCE_DELAY) {
            handleButtonPress();

            // Go back to sleep after handling button press
            delay(3000);  // Wait a bit before sleeping
            enterDeepSleep();
        }
    }

    delay(10);
}

/// @brief The SetButton method registers a press button with 1 pin number.
/// @param id A name for the component, e.g. "my-press-button".
/// @param pin_in A pin number, i.e. to the ESP32 pin number wired to + of the button.
void DoorBell::SetButton(const char* id, unsigned short pin_in) {
    button_ = ButtonDevice{Device<1>{String(id), {pin_in}}};
    has_button_ = true;
}

/// @brief The GetButton method returns a ButtonDevice instance.
/// @return The initialized ultrasonic sensor instance.
ButtonDevice& DoorBell::GetButton() {
    return this->button_;
}

/// @brief The GetName method returns the DoorBell name.
/// @return The name of the DoorBell instance.
const String& DoorBell::GetName() {
    return this->name_;
}

/// @brief The GetVersion method returns the DoorBell version.
/// @return The version of the DoorBell instance.
const String& DoorBell::GetVersion() {
    return this->version_;
}

/// @brief The IsOnline method returns whether the DoorBell is connected to WiFi.
/// @return Returns true if the DoorBell is connected to a WiFi network.
bool DoorBell::IsOnline() {
    return this->online_;
}

/// @brief The GetIPAddress method returns the local IP address if available.
/// @return Returns a local IP address when connected to a WiFi network.
const String &DoorBell::GetIPAddress() {
    return this->ip_address_;
}

/// @brief The setupWiFiConnection method configures the WiFi if needed.
/// @details See also .vscode/arduino.json to update your credentials.
/// CAUTION: WIFI_SSID and WIFI_PASS are defined with -D build flags.
void DoorBell::setupWiFiConnection()
{
    // Read credentials if possible
    settings_.begin("wifi", true);  // true = read only
    String ssid = settings_.getString("ssid", "");
    String password = settings_.getString("password", "");
    settings_.end();

    // Save credentials if necessary
    // See .vscode/arduino.json to update WIFI_SSID, WIFI_PASS.
    if (ssid.isEmpty() && TIMBER_WIFI_SSID != "Default" && TIMBER_WIFI_PASS != "Default") {
        settings_.begin("wifi", false);  // false = read/write
        settings_.putString("ssid", TIMBER_WIFI_SSID);
        settings_.putString("password", TIMBER_WIFI_PASS);
        ssid = String(TIMBER_WIFI_SSID);
        password = String(TIMBER_WIFI_PASS);
        settings_.end();

        sendDebugMessage(String("WiFi credentials saved"));
    }

    // Connect to WiFi if possible
    if (!ssid.isEmpty()) {
        if constexpr(TIMBER_DEBUG_ENABLED) {
            Serial.print("Connecting to WiFi");
        }

        WiFi.begin(ssid.c_str(), password.c_str());

        bool hasError = false;
        bool isOnline = false;
        int  numAttempts = 0;
        while (!isOnline && !hasError && ++numAttempts < 10) {
            if constexpr(TIMBER_DEBUG_ENABLED) {
                Serial.print(".");
            }

            switch (WiFi.status()) {
            case WL_CONNECTED:
                isOnline = true;
            break;

            case WL_CONNECT_FAILED:
            case WL_NO_SSID_AVAIL:
                hasError = true;
            break;
            default:
            break;
            }
            delay(500);
        }

        if (isOnline) {
            sendDebugMessage(String("Connection to WiFi established!"));
            online_ = true;
            ip_address_ = WiFi.localIP().toString();
        } else {
            sendDebugMessage(String("\n[WARN] Could not establish WiFi connection."));
        }
    } else {
        sendDebugMessage(String("[WARN] WiFi is unable to connect: missing credentials."));
    }
}

/// @brief The ensureWiFiConnected method ensures WiFi connectivity.
/// @details This method is called before sending HTTP requests.
void DoorBell::ensureWiFiConnected() {
    if (WiFi.status() == WL_CONNECTED) {
        online_ = true;
        return;
    }

    Serial.println("[WARN] WiFi disconnected, reconnecting...");
    online_ = false;

    WiFi.disconnect();
    setupWiFiConnection();
}

/// @brief The handleButtonPress method sends a HTTP request (webhook).
/// @details See also .vscode/arduino.json to update WEBHOOK_URL.
/// CAUTION: URL_BUTTON_PRESS is defined with -DWEBHOOK_URL build flag.
void DoorBell::handleButtonPress() {
    if (sendHTTPRequest(URL_BUTTON_PRESS)) {
        Serial.println("[INFO] Press notification sent successfully!");
    } else {
        Serial.println("[WARN] Failed to send press notification");
    }
}

/// @brief The enterDeepSleep method shuts down the device.
/// @details This method registers a Wake-On-Button-Press wakeup process.
void DoorBell::enterDeepSleep() {
    Serial.println("[INFO] Entering deep sleep...");
    Serial.println("[INFO] Press button to wake up");

    // Configure wake up on button press (GPIO15 going LOW)
    esp_sleep_enable_ext0_wakeup(GPIO_BUTTON_PIN, 0); // 0 = wake on LOW

    // Disconnect WiFi to save power
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    delay(100);

    // Enter deep sleep
    esp_deep_sleep_start();
}

/// @brief The sendHTTPRequest method sends a HTTP request to a HASS webhook.
/// @details This method ensure WiFi connectivity or fails.
bool DoorBell::sendHTTPRequest(const char* url) {
    // Ensure WiFi is connected
    ensureWiFiConnected();

    if (!online_) {
        Serial.println("[WARN] Cannot call webhook - WiFi is not connected");
        return false;
    }

    Serial.printf("[INFO] Sending HTTP GET request to:\n   %s\n", url);

    HTTPClient http;
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT);

    // Add headers if needed
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device", "timber-doorbell");

    int httpResponseCode = http.POST(String("{}"));
    if (httpResponseCode > 0) {
        Serial.printf("[INFO] HTTP Response code: %d\n", httpResponseCode);

        // Optionally read response body
        if (httpResponseCode == HTTP_CODE_OK) {
            String response = http.getString();
            if (response.length() > 0 && response.length() < 200) {
                Serial.printf("  Response: %s\n", response.c_str());
            }
        }

        http.end();
        return true;
    } else {
        Serial.printf("[WARN] HTTP Error: %s\n", http.errorToString(httpResponseCode).c_str());
        http.end();
        return false;
    }
}
