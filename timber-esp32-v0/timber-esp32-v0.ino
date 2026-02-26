/*
 * ═══════════════════════════════════════════════════════════════════════════
 *  TIMBER - Smart Doorbell v1.0
 * ═══════════════════════════════════════════════════════════════════════════
 *  
 *  Features:
 *  - Short press detection (< 1 second)
 *  - Long press detection (>= 1 second)
 *  - HTTP GET requests to configurable URLs
 *  - Deep sleep between button presses for battery savings
 *  - WiFi connection with auto-reconnect
 *  
 *  Wiring:
 *  - Button: D15 to GND (using internal pull-up)
 *  - Power: AA battery pack to VIN and GND
 *  
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <WiFi.h>
#include <HTTPClient.h>

#include "src/constants.h"
#include "src/door_bell.h"

#define TIMBER_VERSION "0.1.0"

DoorBell* TIMBER;

void setup() {
    Serial.begin(9600);
    delay(2000); // Wait for serial..

    TIMBER = new DoorBell("Timber", TIMBER_VERSION);
    TIMBER->SetButton("button", PRESS_BUTTON_PIN);

    TIMBER->Setup();

    delay(2000);
    Serial.println();
    Serial.println("DoorBell is now waiting for guests...");

    TIMBER->OnWake();
}

void loop() {
    TIMBER->OnLoop();
}
