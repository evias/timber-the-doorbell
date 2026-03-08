// timber-tester/timber-tester.ino
// An arduino sketch for testing the TIMBER ESP32 board firmware.
// Tester for hardware components wiring and custom libraries being used
// by the Timber firmware, e.g. SPIFFS, and WiFi connection.
//
// Copyright 2026 Grégory Saive <greg@evi.as> for re:Software S.L. (resoftware.es).

#include <Arduino.h>
#include <SPIFFS.h>
#include "../timber-esp32-v0/src/constants.h"
#include "../timber-esp32-v0/src/types.h"

/**
 * You may update the interval between serial messages in .vscode/arduino.json.
 * e.g.: `-DLOG_INTERVAL_MS=10000`
 */
#ifndef LOG_INTERVAL_MS
    #define LOG_INTERVAL_MS 30000
#endif

void setup()
{
    Serial.begin(9600);
    delay(4000);

    pinMode(PRESS_BUTTON_PIN, INPUT_PULLUP);
    SPIFFS.begin(true);

    setupGestureSensor();
    delay(1000);
    selectBank(0);
    delay(50);

    delay(2000);
    Serial.println("[OK] Setup done...");

    displayBoardInfo(true);
    testPAJ7620();
}

void loop()
{
    displayBoardInfo(false);
    delay(LOG_INTERVAL_MS);
}

bool setupGestureSensor()
{
    pinMode(GESTURE_SDA_PIN, INPUT_PULLUP);
    pinMode(GESTURE_SCL_PIN, INPUT_PULLUP);
    delay(50);

    // Order here is SDA, SCL
    Wire.begin(GESTURE_SDA_PIN, GESTURE_SCL_PIN);
    Wire.setClock(25000);

    Serial.println("[OK] Gesture sensor initialized");
    return true;
}

bool hasGestureSensor()
{
    Wire.beginTransmission(PAJ7620_CHIP_ADDR);
    int error = Wire.endTransmission();
    Serial.printf("Device 0x%02X response: %d (0=ACK, 2=NACK)\n", PAJ7620_CHIP_ADDR, error);
    return error == 0;
}

void displayBoardInfo(bool header)
{
    if (header) {
        Serial.println();
        Serial.println("=========================================");
        Serial.println("  Timber Components Diagnostic Tool");
        Serial.println("=========================================");
        Serial.println();
    }

    Serial.println();
    Serial.print("Chip Model: ");
    Serial.println(ESP.getChipModel());
    Serial.print("Chip Revision: ");
    Serial.println(ESP.getChipRevision());
    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");
    Serial.print("Flash Size: ");
    Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
    Serial.println(" MB");
    Serial.print("Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");

    uint8_t chipID = readRegister(0x00);
    Serial.printf("PAJ7620 Address: 0x%02X\n", PAJ7620_CHIP_ADDR);
    Serial.printf("PAJ7620 ID: 0x%02X\n", chipID);
    Serial.printf("PAJ7620 Pins: INT: D%d, SDA: D%d, SCL: D%d\n", 
        GESTURE_INT_PIN,
        GESTURE_SDA_PIN,
        GESTURE_SCL_PIN);

    Serial.printf("Press Button Pin: D%d\n", PRESS_BUTTON_PIN);

    Serial.println();
    Serial.printf("IRIS BLE UUID: %s\n", IRISBLE_SERVICE_UUID);
}

void testPAJ7620()
{
    testPAJ7620_availability();
    testPAJ7620_directRead();
    testPAJ7620_selectBank();
    testPAJ7620_directReadAgain();
}

void testPAJ7620_availability()
{
    Serial.println("[TEST 1] Scanning for gesture sensor...");
    if (!hasGestureSensor()) {
        Serial.println("[FATAL] Device not found!");
        return;
    }
}

void testPAJ7620_directRead()
{
    // Test 2: Try simple read without any prior write
    Serial.println("\n[TEST 2] Direct read from 0x00 (no setup)...");
    uint8_t val;
    if (!testHelper_canDirectRead(0x00, &val)) {
        Serial.printf("[FAIL] Could not read 0x00\n");
    } else {
        Serial.printf("[OK] Read 0x00 = 0x%02X (expect 0x20)\n", val);
    }
}

void testPAJ7620_selectBank()
{
    // Test 3: Try bank selection with different approaches
    Serial.println("\n[TEST 3] Attempt bank selection (0xEF = 0)...");
    testHelper_attemptBankSelect();
}

void testPAJ7620_directReadAgain()
{
    // Test 4: Check if device responds after wait
    Serial.println("\n[TEST 4] Long delay then read...");
    delay(2000);
    uint8_t val;
    if (!testHelper_canDirectRead(0x00, &val)) {
        Serial.printf("[FAIL] Still cannot read\n");
    } else {
        Serial.printf("[OK] After delay: 0x00 = 0x%02X\n", val);
    }
}

bool testHelper_canDirectRead(uint8_t reg, uint8_t *value) {
    Serial.printf("  Attempting direct requestFrom(0x%02X)...\n", reg);
    uint8_t bytes = Wire.requestFrom(PAJ7620_CHIP_ADDR, 1, true);
    delay(5);
    if (bytes == 0) {
        Serial.println("  [FAIL] No bytes received");
        return false;
    }

    *value = Wire.read();
    Serial.printf("  [OK] Got 1 byte: 0x%02X\n", *value);
    return true;
}

void testHelper_attemptBankSelect() {
    Serial.println("  Method 1: Simple write to 0xEF = 0");
    Wire.beginTransmission(PAJ7620_CHIP_ADDR);
    Wire.write(0xEF);
    Wire.write(0x00);
    int error = Wire.endTransmission(true);
    Serial.printf("  Result: %d\n", error);
    delay(100);

    Serial.println("  Method 2: With extra delays");
    Wire.beginTransmission(PAJ7620_CHIP_ADDR);
    delay(10);
    Wire.write(0xEF);
    delay(10);
    Wire.write(0x00);
    delay(10);
    error = Wire.endTransmission(true);
    Serial.printf("  Result: %d\n", error);
    delay(100);

    Serial.println("  Method 3: Separate transmissions");
    Wire.beginTransmission(PAJ7620_CHIP_ADDR);
    Wire.write(0xEF);
    error = Wire.endTransmission(true);
    Serial.printf("  Reg select result: %d\n", error);
    delay(50);

    Wire.beginTransmission(PAJ7620_CHIP_ADDR);
    Wire.write(0x00);
    error = Wire.endTransmission(true);
    Serial.printf("  Write value result: %d\n", error);
}

void selectBank(uint8_t bank) {
    writeRegister(0xEF, bank);
    delay(50);  // Wait for bank switch
}

void writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(PAJ7620_CHIP_ADDR);
    Wire.write(reg);
    Wire.write(value);
    int error = Wire.endTransmission(true);
    
    if (error != 0) {
        Serial.printf("[ERROR] Write 0x%02X failed: %d\n", reg, error);
    }
    delay(10);  // Small delay between operations
}

uint8_t readRegister(uint8_t reg) {
    Wire.beginTransmission(PAJ7620_CHIP_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);  // Repeated START
    delay(5);

    Wire.requestFrom(PAJ7620_CHIP_ADDR, 1, true);
    delay(5);

    uint8_t value = Wire.read();
    return value;
}
