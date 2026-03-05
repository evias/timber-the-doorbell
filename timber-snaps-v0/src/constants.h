// SPDX-License-Identifier: BSD-3-Clause
#ifndef TIMBER_SNAPS_CONSTANTS_H
#define TIMBER_SNAPS_CONSTANTS_H

/**
 * You may enable debug mode in .vscode/arduino.json.
 * e.g.: `-DDEBUG_MODE=1`
 */
#ifndef DEBUG_MODE
    #define DEBUG_MODE 0
#endif

constexpr bool TIMBER_SNAPS_DEBUG_ENABLED = (bool) DEBUG_MODE;

/**
 * You may update the UUID namespace. Should contain a 30 bytes UUID,
 * i.e. 34 characters with dashes).
 * e.g.: `-DIRIS_UUID_NS="04a0335e-bea7-429a-50b6-b4621d9570"`
 *
 * CAUTION: If you modify this value, you must update the TIMBER firmware as well.
 */
#ifndef IRIS_UUID_NS
    #define IRIS_UUID_NS "04a0335e-bea7-429a-50b6-b4621d9570";
#endif

/**
 * Bluetooth UUIDs (BLE) for the Camera device.
 * UUID namespace uses first 30 bytes of SHA1 hash as illustrated.
 *
 * ```python
 * # results in: 04a0335ebea7429a50b6b4621d957099c68e08f3 (40 bytes)
 * doc = "github.com/evias/timber-the-doorbell"
 * ns = hash(doc)[:30] # Use first 30 bytes
 * ```
 */

#define IRISBLE_SERVICE_UUID   IRIS_UUID_NS "00" // BLE Connect (Service)
#define IRISBLE_TRIGGER_UUID   IRIS_UUID_NS "01" // Trigger Snapshot
#define IRISBLE_SIZE_UUID      IRIS_UUID_NS "02" // Image Size
#define IRISBLE_DATA_UUID      IRIS_UUID_NS "03" // Image Data

/**
 * Camera pins for ESP32-CAM-MB
 */
#define IRISCAM_PWDN_GPIO_NUM     32
#define IRISCAM_RESET_GPIO_NUM    -1
#define IRISCAM_XCLK_GPIO_NUM      0
#define IRISCAM_Y9_GPIO_NUM       35
#define IRISCAM_Y8_GPIO_NUM       34
#define IRISCAM_Y7_GPIO_NUM       39
#define IRISCAM_Y6_GPIO_NUM       36
#define IRISCAM_Y5_GPIO_NUM       21
#define IRISCAM_Y4_GPIO_NUM       19
#define IRISCAM_Y3_GPIO_NUM       18
#define IRISCAM_Y2_GPIO_NUM        5
#define IRISCAM_VSYNC_GPIO_NUM    25
#define IRISCAM_HREF_GPIO_NUM     23
#define IRISCAM_PCLK_GPIO_NUM     22

#endif