// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_CONSTANTS_H
#define TIMBER_CONSTANTS_H

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
    #define WEBHOOK_URL "http://localhost:8123/api/webhook/doorbell";
#endif

constexpr bool TIMBER_DEBUG_ENABLED = (bool) DEBUG_MODE;
constexpr char TIMBER_WIFI_SSID[] = WIFI_SSID;
constexpr char TIMBER_WIFI_PASS[] = WIFI_PASS;

/**
 * Timing Configuration (in milliseconds)
 */
constexpr char URL_BUTTON_PRESS[] = WEBHOOK_URL;
constexpr unsigned long DEBOUNCE_DELAY = 50;   // Debounce time
constexpr unsigned long HTTP_TIMEOUT   = 5000; // 5 seconds HTTP request timeout

/**
 * 2-pin Press Button
 *
 * esp32(D15) -> B(1)
 * esp32(GND) -> B(2)
 **/
const unsigned short PRESS_BUTTON_PIN  = 15; // D15
const gpio_num_t GPIO_BUTTON_PIN = GPIO_NUM_15;

#endif