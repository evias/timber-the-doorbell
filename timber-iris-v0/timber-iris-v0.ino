/*
 * ═══════════════════════════════════════════════════════════════════════════
 *  Iris of Tímber - Camera over BLE v1.0
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *  Features:
 *  - Connectable over BLE (Bluetooth Low Energy)
 *  - Capture triggering over BLE characteristics
 *  - Size-prefixed Capture (Image data) Transmission
 *
 * a.k.a: "Iris-of-Timber" or `04a0335e-bea7-429a-50b6-b4621d957000`.
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <BLEDevice.h>
#include <BLE2902.h>

#include "src/constants.h"
#include "src/camera.h"

Camera* IRIS;

void setup() {
    Serial.begin(9600);
    delay(2000); // Wait for serial..

    IRIS = new Camera("Iris", IRIS_VERSION);
    IRIS->Setup();

    delay(2000);
    Serial.println();
    Serial.printf("[OK] Firmware: IRIS@%s\n", IRIS_VERSION);
}

void loop() {
    IRIS->OnLoop();
}
