// SPDX-License-Identifier: BSD-3-Clause
#include <Arduino.h>    // Serial, String, digitalWrite, pinMode
#include "esp_camera.h" // esp_camera_init
#include <BLE2902.h>

#include "camera.h"
#include "constants.h"
#include "serial.h"
#include "bluetooth.h"

/// @brief DoorBell public constructor, says Hi.
/// @param name A friendly name for your DoorBell, e.g. "Tímber".
/// @param version A version number, recommended semantic versioning, e.g. "1.0.0".
Camera::Camera(const char* name, const char* version)
    : name_(String(name)),
      version_(String(version))
{
    sendDebugMessage(String("Hola! I am Iris, the eye of Timber ;]"));
}

/// @brief The Setup method initializes configured pin numbers.
/// @details This method is called from the sketch's setup() function.
void Camera::Setup()
{
    if (!setupCameraDevice()) {
        Serial.println("[WARN] Camera init failed");
        return;
    }
    sendDebugMessage(String("Camera device initialized"));

    setupBluetoothServer();
    sendDebugMessage(String("BLE server started"));

    setupBluetoothAdvert();
    sendDebugMessage(String("BLE advertising started"));

    sendDebugMessage(String("Camera setup completed"));
    sendDebugMessage(String("Camera BLE UUID: ") + IRISBLE_SERVICE_UUID);
}

/// @brief The OnLoop method is basically a no-op for the Iris implementation.
/// @details This method is called from the sketch's loop() function.
void Camera::OnLoop()
{
    delay(100); // breathe...
}

/// @brief The setupCameraDevice method configures the camera device (ESP32-CAM).
/// @return Returns true if the configuration returns `ESP_OK`.
bool Camera::setupCameraDevice()
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

/// @brief The setupBluetoothServer method configures the BLE server.
/// @details This method registers BLE characteristics for triggering a capture,
///          and for transmitting the image- size and data. It should start BLE.
void Camera::setupBluetoothServer()
{
    // Initialize BLE
    BLEDevice::init("Iris-of-Timber");
    ble_server_ = BLEDevice::createServer();
    ble_server_->setCallbacks(new IrisServerCallbacks(*this));

    // Create service
    ble_service_ = ble_server_->createService(IRISBLE_SERVICE_UUID);

    // Create characteristics
    trigger_op_ = ble_service_->createCharacteristic(
        IRISBLE_TRIGGER_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    trigger_op_->setCallbacks(new IrisCharacteristicsCallbacks(*this));

    size_op_ = ble_service_->createCharacteristic(
        IRISBLE_SIZE_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    size_op_->addDescriptor(new BLE2902());

    data_op_ = ble_service_->createCharacteristic(
        IRISBLE_DATA_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    data_op_->addDescriptor(new BLE2902());

    // Start service
    ble_service_->start();
}

/// @brief The setupBluetoothAdvert method configures BLE advertising.
void Camera::setupBluetoothAdvert()
{
  // Start advertising
  BLEAdvertising *advert_op = BLEDevice::getAdvertising();
  advert_op->addServiceUUID(IRISBLE_SERVICE_UUID);
  advert_op->setScanResponse(true);
  advert_op->setMinPreferred(0x06); // 7.5ms
  advert_op->setMaxPreferred(0x12); // 22.5ms
  BLEDevice::startAdvertising();
}
