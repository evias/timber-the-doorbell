// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_SNAPS_BLUETOOTH_H
#define TIMBER_SNAPS_BLUETOOTH_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "esp_camera.h" // esp_camera_x
#include "constants.h"
#include "serial.h"
#include "camera.h"

class IrisServerCallbacks
    : public BLEServerCallbacks
{
    Camera& camera_;

    void onConnect(BLEServer* pServer)
    {
        camera_.online_ = true;
        sendDebugMessage(String("BLE Client connected"));
    }

    void onDisconnect(BLEServer* pServer)
    {
        camera_.online_ = false;
        sendDebugMessage(String("BLE Client disconnected"));
        BLEDevice::startAdvertising();
    }

public:
    IrisServerCallbacks(Camera& c) : camera_(c) {}
    ~IrisServerCallbacks() {}
};

class IrisCharacteristicsCallbacks
    : public BLECharacteristicCallbacks
{
    Camera& camera_;

    void onWrite(BLECharacteristic *op)
    {
        String value = op->getValue();

        // We should only reactor to TRIGGER operation.
        if (value.length() == 0 || value[0] != 1) {
            return ;
        }

        sendDebugMessage(String("Capture triggered by client"));

        // Release old frame if exists
        if (camera_.current_frame_) {
            esp_camera_fb_return(camera_.current_frame_);
        }

        // Capture new frame
        camera_.current_frame_ = esp_camera_fb_get();
        if (!camera_.current_frame_) {
            sendDebugMessage(String("[WARN] Camera capture failed"));
            return;
        }

        // Send image size first
        uint32_t imageSize = camera_.current_frame_->len;
        camera_.size_op_->setValue((uint8_t*)&imageSize, 4);
        camera_.size_op_->notify();

        // Send image data in chunks (512 bytes per chunk)
        const uint16_t CHUNK_SIZE = 512;
        for (uint32_t i = 0; i < imageSize; i += CHUNK_SIZE) {
            uint16_t chunkLen = (imageSize - i) > CHUNK_SIZE ? CHUNK_SIZE : (imageSize - i);
            camera_.data_op_->setValue((camera_.current_frame_->buf) + i, chunkLen);
            camera_.data_op_->notify();

            delay(10); // Small delay between chunks
        }

        sendDebugMessage(String("Image data transmitted"));
    }

public:
    IrisCharacteristicsCallbacks(Camera& c) : camera_(c) {}
    ~IrisCharacteristicsCallbacks() {}
};

#endif