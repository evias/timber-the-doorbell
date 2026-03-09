# Tímber: Someone's at the door!

A tiny (over-engineered) *smart door bell* project. This repository contains the
source code (C++) for:

- **Tímber**: A `ESP32-DEVKIT-V1` development board: [`timber-esp32-v0`](./timber-esp32-v0).
- **Tímber tester**: `timber-esp32-tester/timber-esp32-tester.ino`
- **Iris**: A `ESP32-CAM-MB` camera board: [`timber-iris-v0`](./timber-iris-v0).
- **Iris tester**: `timber-iris-tester/timber-iris-tester.ino`

## Developer notes

This repository contains arduino sketches that may be built/uploaded using Arduino IDE
or VSCode. I personally use VSCode with the added `vscode-arduino` community extension.

When building with VSCode, the `.vscode/arduino.json` must be updated to reflect
the build configuration for each *separate* arduino sketch, e.g.:

```bash
# Prepare for building the TIMBER firmware (ESP32-DEVKIT-V1).
cp .vscode/arduino.timber.json .vscode/arduino.json

# OR prepare for building the IRIS firmware (ESP32-CAM-MB).
cp .vscode/arduino.iris.json .vscode/arduino.json
```

After having done that, enable the `vscode-arduino` extension for your workspace
and use the `Arduino: Verify` and `Arduino: Upload` actions from the command
palette to build and/or upload the firmware to your chosen board.

The default `.vscode/arduino.json` file contains building instruction for `TIMBER`.

## 3D Models

This project includes 3D-printed components.

Refer to each sketch's README for more details:

- [`Door bell 3D Models`](./timber-esp32-v0/README.md#3d-printing)
- [`Camera 3D Models`](./timber-iris-v0/README.md#3d-printing)

## License

Copyright 2026 Grégory Saive <greg@evi.as> for re:Software S.L. (resoftware.es).

Licensed under the [3-Clause BSD License](./LICENSE).
