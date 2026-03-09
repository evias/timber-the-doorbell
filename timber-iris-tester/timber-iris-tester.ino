// timber-iris-tester/timber-iris-tester.ino
// An arduino sketch for testing the IRIS ESP32-CAM board firmware.
// Tester for hardware components wiring and custom libraries being used
// by the Iris firmware, e.g. SPIFFS, and BLE server.
//
// Copyright 2026 Grégory Saive <greg@evi.as> for re:Software S.L. (resoftware.es).

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "esp_camera.h" // esp_camera_init

#include "../timber-iris-v0/src/constants.h"
#include "bluetooth.h"

/**
 * You may update the interval between serial messages in .vscode/arduino.json.
 * e.g.: `-DLOG_INTERVAL_MS=10000`
 */
#ifndef LOG_INTERVAL_MS
    #define LOG_INTERVAL_MS 30000
#endif

BLECharacteristic*   trigger_op;
BLECharacteristic*      size_op;
BLECharacteristic*      data_op;
BLEServer*           ble_server;
BLEService*         ble_service;

void setup()
{
    Serial.begin(9600);
    delay(4000);

    if (!setupCameraDevice()) {
        Serial.println("[WARN] Camera init failed");
        return;
    }
    Serial.println("[OK] Camera device initialized");

    setupBluetoothServer();
    Serial.println("[OK] BLE server started");

    setupBluetoothAdvert();
    Serial.println("[OK] BLE advertising started");

    delay(2000);

    displayBoardInfo(true);

    Serial.println();
    Serial.printf("Firmware: IRIS.tester@%s\n", IRIS_VERSION);
}

void loop()
{
    displayBoardInfo(false);
    delay(LOG_INTERVAL_MS);
}

void displayBoardInfo(bool header)
{
    if (header) {
        Serial.println();
        Serial.println("=========================================");
        Serial.println("  Iris Components Diagnostic Tool");
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
    Serial.printf("Firmware: IRIS.tester@%s\n", IRIS_VERSION);

    Serial.println();
    Serial.printf("IRIS BLE UUID: %s\n", IRISBLE_SERVICE_UUID);
}

bool setupCameraDevice()
{
    camera_config_t config;
    config.pin_d0 = IRISCAM_Y2_GPIO_NUM;
    config.pin_d1 = IRISCAM_Y3_GPIO_NUM;
    config.pin_d2 = IRISCAM_Y4_GPIO_NUM;
    config.pin_d3 = IRISCAM_Y5_GPIO_NUM;
    config.pin_d4 = IRISCAM_Y6_GPIO_NUM;
    config.pin_d5 = IRISCAM_Y7_GPIO_NUM;
    config.pin_d6 = IRISCAM_Y8_GPIO_NUM;
    config.pin_d7 = IRISCAM_Y9_GPIO_NUM;
    config.pin_xclk = IRISCAM_XCLK_GPIO_NUM;
    config.pin_pclk = IRISCAM_PCLK_GPIO_NUM;
    config.pin_vsync = IRISCAM_VSYNC_GPIO_NUM;
    config.pin_href = IRISCAM_HREF_GPIO_NUM;
    config.pin_pwdn = IRISCAM_PWDN_GPIO_NUM;
    config.pin_reset = IRISCAM_RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    return esp_camera_init(&config) == ESP_OK;
}

void setupBluetoothServer()
{
    // Initialize BLE
    BLEDevice::init("Iris-of-Timber-TESTER");
    ble_server = BLEDevice::createServer();
    ble_server->setCallbacks(new TestServerCallbacks());

    // Create service
    ble_service = ble_server->createService(IRISBLE_SERVICE_UUID);

    // Create characteristics
    trigger_op = ble_service->createCharacteristic(
        IRISBLE_TRIGGER_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    trigger_op->setCallbacks(new TestCharacteristicsCallbacks());

    size_op = ble_service->createCharacteristic(
        IRISBLE_SIZE_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    size_op->addDescriptor(new BLE2902());

    data_op = ble_service->createCharacteristic(
        IRISBLE_DATA_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    data_op->addDescriptor(new BLE2902());

    // Start service
    ble_service->start();
}

void setupBluetoothAdvert()
{
  // Start advertising
  BLEAdvertising *advert_op = BLEDevice::getAdvertising();
  advert_op->addServiceUUID(IRISBLE_SERVICE_UUID);
  advert_op->setScanResponse(true);
  advert_op->setMinPreferred(0x06); // 7.5ms
  advert_op->setMaxPreferred(0x12); // 22.5ms
  BLEDevice::startAdvertising();
}
