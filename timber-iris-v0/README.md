# Iris-of-Timber: Look who's at the door!

An addon project to Tímber which programs a ESP32-CAM-MB device. This sketch
contains the firmware source code (C++) for a ESP32-CAM-MB board.

Refer to [3D Printing](#3d-printing) for the model files.

## Developer notes

This sketch contains may be built/uploaded using Arduino IDE or VSCode.
I personally use VSCode with the added `vscode-arduino` community extension.

When building with VSCode, the `.vscode/arduino.json` must be updated to reflect
the build configuration for the **IRIS firmware**, i.e.:

```bash
# Prepare for building the IRIS firmware (ESP32-CAM-MB).
cp .vscode/arduino.iris.json .vscode/arduino.json
```

After having done that, enable the `vscode-arduino` extension for your workspace
and use the `Arduino: Verify` and `Arduino: Upload` actions from the command
palette to build and/or upload the firmware to your chosen board.

The default `.vscode/arduino.json` file contains building instruction for `TIMBER`.


## Implementation notes

- Main sketch file: [`timber-iris-v0.ino`](./timber-iris-v0.ino)
  - Simple interface to Arduino which creates a global `Camera` instance.
- `Camera` class: [`src/camera.h`](./src/camera.h)
  - Handles the setup/configuration of the camera device.
  - Used as a wrapper to initialize a BLE server and characteristics.
- `IrisServerCallbacks` class: [`src/bluetooth.h`](./src/bluetooth.h)
  - Simple `onConnect` and `onDisconnect` with logging.
  - `Camera` class is befriended for write access to `Camera::online_`.
- `IrisCharacteristicsCallbacks` class: [`src/bluetooth.h`](./src/bluetooth.h)
  - Interprets **trigger operation** (see `IRISBLE_TRIGGER_UUID`) over BLE.
  - `Camera` class is befriended for write access to `Camera::current_frame_`.
- Callback classes implement BLE operations as defined with `<BLEServer.h>`.

## 3D Printing

This project includes 3D-printed components.

### Original models

https://www.printables.com/@aqab0N_584411

| 3D Model | Attribution | Description |
| --- | --- | --- |
| [ESP32 Cam Case](https://www.printables.com/model/424005-esp32-cam-case) | [@aqab0N_584411](https://www.printables.com/@aqab0N_584411) | A practical ESP32 camera case. |

## Hardware

| Component | Device | Marketplace |
| --- | --- | --- |
| 1x Camera + development board | ESP32-CAM-MB (micro USB) | [AliExpress](https://es.aliexpress.com/item/1005010579171124.html) |

## License

Copyright 2026 Grégory Saive <greg@evi.as> for re:Software S.L. (resoftware.es).

Licensed under the [3-Clause BSD License](./LICENSE).
