// SPDX-License-Identifier: BSD-3-Clause
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_sleep.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

#include "door_bell.h"
#include "constants.h"
#include "serial.h"
#include "camera.h"

File DoorBell::snapshot_;
uint32_t DoorBell::snap_bytes_ = 0;
uint32_t DoorBell::rcvd_bytes_ = 0;

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
      btn_active_(false),
      ble_connected_(false),
      started_at_(0),
      last_activity_at_(0),
      btn_press_at_(0),
      btn_release_at_(0),
      ble_snapshot_at_(0),
      ble_lastscan_at_(0)
{
    sendDebugMessage(String("Hola! I am Timber :]"));
}

/// @brief The Setup method initializes configured pin numbers.
/// @details This method is called from the sketch's setup() function.
void DoorBell::Setup()
{
    // Configure button pin and SPIFFS
    if (has_button_) {
        pinMode(button_.dev.pins[1], INPUT_PULLUP);
    }
    SPIFFS.begin(true);

    int attempts = 0;
    while (++attempts <= 5 && !setupGestureSensor()) {
        Serial.println(String("Waiting for gesture sensor"));
        delay(100);
    }

    setupWiFiConnection();
    setupCameraConnection();

    sendDebugMessage(String("DoorBell setup [OK]"));
}

/// @brief The OnWake method identifies the wakeup cause, if it's boot do nothing.
/// @details This method is called from the sketch's setup() function and makes
///          sure that given a wakeup-by-button, the button press is also handled.
void DoorBell::OnWake()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            last_activity_at_ = millis();
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
void DoorBell::OnLoop()
{
    unsigned long uptime_ms = millis();
    if (started_at_ == 0) {
        started_at_ = uptime_ms;
        last_activity_at_ = started_at_;
    }

    bool  old_ = btn_state_;
    btn_state_ = digitalRead(PRESS_BUTTON_PIN);

    if (old_ != btn_state_) {
        last_activity_at_ = uptime_ms;

        switch(btn_state_) {
        default:
        case LOW: // button pressed
            btn_press_at_ = uptime_ms;
            btn_active_ = true;
            Serial.println(String("[v] Button pressed..."));
            break;
        case HIGH: // button released
            btn_release_at_ = uptime_ms;
            btn_active_ = false;
            if (btn_release_at_ - btn_press_at_ >= DEBOUNCE_DELAY) {
                handleButtonPress();
            }
            break;
        }
    }

    handleGestureDetection();
    delay(10);

    // Re-scan for Iris-of-Timber if necessary
    if (!ble_connected_ && (
        ble_lastscan_at_ == 0 || uptime_ms >= ble_lastscan_at_ + BLE_SCAN_INTERVAL_MS
    )) {
        sendDebugMessage(String("[BLE] Scanning for Iris-of-Timber..."));
        BLEDevice::getScan()->start(5, false);
        ble_lastscan_at_ = uptime_ms;
        last_activity_at_ = ble_lastscan_at_;
        delay(100);
    }

    // go-to-sleep after 15 seconds of inactivity
    uptime_ms = millis();
    if (uptime_ms >= last_activity_at_ + UPTIME_MAX_MS) {
        enterDeepSleep();
    }
}

void DoorBell::OnData(BLERemoteCharacteristic* op, uint8_t* data, size_t length, bool done)
{
    if (op->getUUID().toString() == IRISBLE_SIZE_UUID) {
        snap_bytes_ = *(uint32_t*)data;
        rcvd_bytes_ = 0;

        sendDebugMessage(String("[BLE] Image size (bytes): ") + String(snap_bytes_));

        SPIFFS.remove("/snapshot.jpg");
        snapshot_ = SPIFFS.open("/snapshot.jpg", FILE_WRITE);
        if (!snapshot_) {
            Serial.println(String("[WARN] File creation failed"));
        }
    }
    else if (op->getUUID().toString() == IRISBLE_DATA_UUID) {
        if (snapshot_) {
            snapshot_.write(data, length);
            rcvd_bytes_ += length;

            Serial.print(".");

            if (rcvd_bytes_ >= snap_bytes_) {
                Serial.print("\n");

                snapshot_.close();
                Serial.println(String("[INFO] Image saved (SPIFFS)!"));
            }
        }
    }

    /// @todo Should send image data to HASS
}

/// @brief The SetButton method registers a press button with 1 pin number.
/// @param id A name for the component, e.g. "my-press-button".
/// @param pin_in A pin number, i.e. the ESP32 pin number wired to + of the button.
void DoorBell::SetButton(const char* id, unsigned short pin_in)
{
    button_ = ButtonDevice{Device<1>{String(id), {pin_in}}};
    has_button_ = true;
}

/// @brief The SetSensor method registers a 3D gesture sensor with 3 (data) pins.
/// @param id A name for the component, e.g. "my-gesture-device".
/// @param pin_int A pin number, i.e. he ESP32 pin number wired to INT of the sensor.
/// @param pin_sda A pin number, i.e. he ESP32 pin number wired to SDA of the sensor.
/// @param pin_scl A pin number, i.e. he ESP32 pin number wired to SCL of the sensor.
void DoorBell::SetSensor(
    const char*id,
    unsigned short pin_int,
    unsigned short pin_sda,
    unsigned short pin_scl
) {
    gesture_ = GestureDevice{Device<3>{String(id), {pin_int, pin_sda, pin_scl}}};
    gesture_.addr = PAJ7620_CHIP_ADDR;
    has_sensor_ = true;
}

/// @brief The GetButton method returns a ButtonDevice instance.
/// @return The initialized press button ButtonDevice instance.
ButtonDevice& DoorBell::GetButton() {
    return this->button_;
}

/// @brief The GetSensor method returns a GestureDevice instance.
/// @return The initialized gesture sensor GestureDevice instance.
GestureDevice& DoorBell::GetSensor() {
    return this->gesture_;
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

/// @brief The setupGestureSensor method configures the gesture sensor device.
bool DoorBell::setupGestureSensor()
{
    // Order here is SDA, SCL
    Wire.begin(gesture_.dev.pins[1], gesture_.dev.pins[2]);
    Wire.setClock(400000);
    delay(50);

    // Wake up the gesture sensor
    Wire.beginTransmission(PAJ7620_CHIP_ADDR);
    Wire.endTransmission();
    delay(5);
    Wire.beginTransmission(PAJ7620_CHIP_ADDR);
    Wire.endTransmission();

    // Verify chip
    gesture_.Write(PAJ7620_BANK_SEL, 0);
    if(gesture_.Read(0x00) != 0x20) return false;

    // Load init
    for(uint16_t i = 0; i < sizeof(PAJ7620_INIT_SEQUENCE)/2; i++) {
        gesture_.Write(
            pgm_read_byte(&PAJ7620_INIT_SEQUENCE[i][0]),
            pgm_read_byte(&PAJ7620_INIT_SEQUENCE[i][1]));
    }
    gesture_.Write(PAJ7620_BANK_SEL, 0);
    return true;
}

/// @brief The setupCameraConnection method configures the BLE client to the cam.
void DoorBell::setupCameraConnection()
{
    BLEDevice::init("Timber");
    BLEScan* scan = BLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new CameraCallbacks(*this));
    scan->setInterval(1100);
    scan->setWindow(1100);
    scan->start(5, false);
    ble_lastscan_at_ = millis();

    sendDebugMessage(String("[BLE] Scanning for Iris-of-Timber..."));
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
            Serial.print(String("Connecting to WiFi")); // don't \n!
        }

        WiFi.begin(ssid.c_str(), password.c_str());

        bool hasError = false;
        bool isOnline = false;
        int  numAttempts = 0;
        while (!isOnline && !hasError && ++numAttempts < 10) {
            if constexpr(TIMBER_DEBUG_ENABLED) {
                Serial.print(String(".")); // don't \n!
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
void DoorBell::ensureWiFiConnected()
{
    if (WiFi.status() == WL_CONNECTED) {
        online_ = true;
        return;
    }

    Serial.println(String("[WARN] WiFi disconnected, reconnecting..."));
    online_ = false;

    WiFi.disconnect();
    setupWiFiConnection();
}

/// @brief The enterDeepSleep method shuts down the device.
/// @details This method registers a Wake-On-Button-Press wakeup process.
void DoorBell::enterDeepSleep()
{
    Serial.println(String("[INFO] Entering deep sleep..."));
    Serial.println(String("[INFO] Press button to wake up"));

    // Configure wake up on button press (GPIO15 going LOW)
    esp_sleep_enable_ext0_wakeup(GPIO_BUTTON_PIN, 0); // 0 = wake on LOW

    // Disconnect WiFi to save power
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    delay(100);

    // Enter deep sleep
    esp_deep_sleep_start();
}

/// @brief The handleButtonPress method sends a HTTP request (webhook).
/// @details See also .vscode/arduino.json to update WEBHOOK_URL.
/// CAUTION: URL_BUTTON_PRESS is defined with -DWEBHOOK_URL build flag.
void DoorBell::handleButtonPress()
{
    if (sendHTTPRequest(URL_BUTTON_PRESS)) {
        Serial.println(String("[INFO] Press notification sent successfully!"));
    } else {
        Serial.println(String("[WARN] Failed to send press notification"));
    }

    requestCameraSnapshot();
}

/// @brief The handleGestureDetection method detects gestures using a PAJ7260 chip.
void DoorBell::handleGestureDetection()
{
    bool has_gesture = false;
    String direction = "";

    uint8_t res = gesture_.Read(PAJ7620_GESTURE_REG);
    switch(res) {
    case 0x01: direction = "RIGHT"; has_gesture = true; break;
    case 0x02: direction = "LEFT"; has_gesture = true;  break;
    case 0x04: direction = "UP"; has_gesture = true;    break;
    case 0x08: direction = "DOWN"; has_gesture = true;  break;
    case 0x10: direction = "NEAR"; has_gesture = true;  break;
    case 0x20: direction = "FAR"; has_gesture = true;   break;
    case 0x40: direction = "CW"; has_gesture = true;    break; // clockwise
    case 0x80: direction = "CCW"; has_gesture = true;   break; // counter clockwise
    default:
        break;
    }
    delay(50);

    if (!has_gesture) {
        return ;
    }

    sendDebugMessage(String("[SENSOR] Detected gesture: " + direction));

    unsigned long uptime_ms = millis();
    last_activity_at_ = uptime_ms;
    if (ble_snapshot_at_ == 0 || uptime_ms > ble_snapshot_at_ + SNAPSHOT_INTERVAL_MS) {
        requestCameraSnapshot();
    }
}

/// @brief The requestCameraSnapshot method sends a "trigger capture" operation.
/// @details This method usees the BLE trigger characteristic which must be
///          registered and in-sync with the Iris firmware UUIDs.
void DoorBell::requestCameraSnapshot()
{
    if (ble_trigger_op_ == nullptr) {
        Serial.println(String("[WARN] Trigger operation is not available"));
        return ;
    }

    sendDebugMessage(String("[BLE] Requesting camera snapshot"));

    uint8_t trigger = 1;
    ble_trigger_op_->writeValue(&trigger, 1);
    ble_snapshot_at_ = millis();
}

/// @brief The sendHTTPRequest method sends a HTTP request to a HASS webhook.
/// @details This method ensure WiFi connectivity or fails.
bool DoorBell::sendHTTPRequest(const char* url)
{
    // Ensure WiFi is connected
    ensureWiFiConnected();

    if (!online_) {
        Serial.println(String("[WARN] Cannot call webhook - WiFi is not connected"));
        return false;
    }

    Serial.print(String("[INFO] Sending HTTP GET request to:\n   "));
    Serial.print(String(url));
    Serial.print(String("\n"));

    HTTPClient http;
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT);

    // Add headers if needed
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device", "timber-doorbell");

    int httpResponseCode = http.POST(String("{}"));
    if (httpResponseCode > 0) {
        Serial.print(String("[INFO] HTTP Response code: "));
        Serial.print(String(httpResponseCode));
        Serial.print(String("\n"));

        // Optionally read response body
        if (httpResponseCode == HTTP_CODE_OK) {
            String response = http.getString();
            if (response.length() > 0 && response.length() < 200) {
                Serial.print(String("  Response: "));
                Serial.print(String(response.c_str()));
                Serial.print(String("\n"));
            }
        }

        http.end();
        return true;
    } else {
        Serial.print(String("[WARN] HTTP Error: "));
        Serial.print(http.errorToString(httpResponseCode));
        Serial.print(String("\n"));
        http.end();
        return false;
    }
}
