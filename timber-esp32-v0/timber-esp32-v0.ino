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

#define TIMBER_VERSION "0.2.0"

DoorBell* TIMBER;

void setup() {
    Serial.begin(9600);
    delay(2000); // Wait for serial..

    TIMBER = new DoorBell("Timber", TIMBER_VERSION);
    TIMBER->SetButton("button", PRESS_BUTTON_PIN);
    TIMBER->SetSensor("gesture", GESTURE_INT_PIN, GESTURE_SDA_PIN, GESTURE_SCL_PIN);
    TIMBER->Setup();

    delay(2000);
    Serial.println();
    Serial.println(F("DoorBell [OK]"));

    TIMBER->OnWake();
}

void loop() {
    TIMBER->OnLoop();
}
