// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_CONSTANTS_H
#define TIMBER_CONSTANTS_H

#define TIMBER_VERSION "0.3.0"

/**
 * Don't forget to change the WiFi SSID in .vscode/arduino.json.
 * e.g.: `-DWIFI_SSID="YourWiFiNetwork"`
 */
#ifndef WIFI_SSID
    #define WIFI_SSID "Default"
#endif

/**
 * Don't forget to change the WiFi password in .vscode/arduino.json.
 * e.g.: `-DWIFI_PASS="YourWiFiPassword"`
 */
#ifndef WIFI_PASS
    #define WIFI_PASS "Default"
#endif

/**
 * You may enable debug mode in .vscode/arduino.json.
 * e.g.: `-DDEBUG_MODE=1`
 */
#ifndef DEBUG_MODE
    #define DEBUG_MODE 0
#endif

/**
 * Don't forget to change the HomeAssistant Webhook URL in .vscode/arduino.json.
 * e.g.: `-DWEBHOOK_URL="http://homeassistant.local:8123/api/webhook/doorbell"`
 */
#ifndef WEBHOOK_URL
    #define WEBHOOK_URL "http://localhost:8123/api/webhook/doorbell"
#endif

constexpr bool TIMBER_DEBUG_ENABLED = (bool) DEBUG_MODE;
constexpr char TIMBER_WIFI_SSID[] = WIFI_SSID;
constexpr char TIMBER_WIFI_PASS[] = WIFI_PASS;
constexpr char URL_BUTTON_PRESS[] = WEBHOOK_URL;

/**
 * You may update the UUID namespace. Should contain a 30 bytes UUID,
 * i.e. 34 characters with dashes).
 * e.g.: `-DIRIS_UUID_NS="04a0335e-bea7-429a-50b6-b4621d9570"`
 *
 * CAUTION: If you modify this value, you must update the IRIS firmware as well.
 */
#ifndef IRIS_UUID_NS
    #define IRIS_UUID_NS "04a0335e-bea7-429a-50b6-b4621d9570"
#endif

/**
 * Iris-of-Timber BLE service details.
 * IMPORTANT: These *must* match the Iris firmware.
 */
#define IRISBLE_SERVICE_UUID   IRIS_UUID_NS "00" // BLE Connect (Service)
#define IRISBLE_TRIGGER_UUID   IRIS_UUID_NS "01" // Trigger Snapshot
#define IRISBLE_SIZE_UUID      IRIS_UUID_NS "02" // Image Size
#define IRISBLE_DATA_UUID      IRIS_UUID_NS "03" // Image Data

/**
 * Timing Configuration (in milliseconds)
 */
constexpr unsigned long DEBOUNCE_DELAY = 50;    // Debounce time
constexpr unsigned long HTTP_TIMEOUT   = 5000;  // 5 seconds HTTP request timeout
constexpr unsigned long UPTIME_MAX_MS  = 15000; // 15 seconds max uptime
constexpr unsigned long BLE_SCAN_INTERVAL_MS = 5000; // 5 seconds interval
constexpr unsigned long SNAPSHOT_INTERVAL_MS = 3000; // 3 seconds interval

/**
 * 2-pin Press Button
 *
 * esp32(D15) -> B(1)
 * esp32(GND) -> B(2)
 **/
const unsigned short PRESS_BUTTON_PIN  = 15; // D15
const gpio_num_t GPIO_BUTTON_PIN = GPIO_NUM_15;

#endif