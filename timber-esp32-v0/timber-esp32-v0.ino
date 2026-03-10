/*
 * ═══════════════════════════════════════════════════════════════════════════
 *  Tímber - Smart Doorbell v1.0
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *  Features:
 *  - HTTP GET requests to configurable URLs
 *  - Deep sleep between button presses for battery savings
 *  - WiFi connection with auto-reconnect
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <SPIFFS.h>

#include "src/constants.h"
#include "src/door_bell.h"

DoorBell* TIMBER;

void setup() {
    Serial.begin(9600);
    delay(2000); // Wait for serial..

    TIMBER = new DoorBell("Timber", TIMBER_VERSION);
    TIMBER->SetButton("button", PRESS_BUTTON_PIN);
    TIMBER->Setup();

    delay(2000);
    Serial.println();
    Serial.printf("[OK] Firmware: TIMBER@%s\n", TIMBER_VERSION);

    TIMBER->OnWake();
}

void loop() {
    TIMBER->OnLoop();
}
