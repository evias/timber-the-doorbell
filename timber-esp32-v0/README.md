# Tímber: Someone's at the door!

A tiny *smart door bell* project. This sketch contains the firmware source code
(C++) for a ESP32 DEVKIT V1 board which is wired together with listed components. 

Note that for outside-use, the ESP32 board is plugged on a `2x AA` batteries pack
on the bottom plate of the esp32 box.
Refer to [3D Printing](#3d-printing) for the model files.

## Implementation notes

- Main sketch file: [`timber-esp32-v0.ino`](./timber-esp32-v0.ino)
  - Simple interface to Arduino which creates a global `DoorBell` instance.
  - Button press is processed in `handleButtonPress`.
- `DoorBell` class: [`src/door_bell.h`](./src/door_bell.h)
  - Handles the setup/configuration of devices and protocols (serial).
  - Used as a wrapper to retrieve individual device struct objects.
- `Device` struct: [`src/types.h`](./src/types.h)
  - A **simplistic** hardware-to-software wrapper to configure hardware mapping.
  - i.e. The `pins` property in `Device` contains the number of pins that are
    wired (or soldered) to a ESP32 pin. e.g. a `Device<2>` contains 2 pins that
    may be used with `digitalWrite` or `analogWrite`; whereas a `Device<1>`
    contains 1 pin. Generally speaking, don't count voltage and grounding pins.

## 3D Printing

This project includes 3D-printed components.

### Original models

| 3D Model | Attribution | Description |
| --- | --- | --- |
| [esp32 box](https://www.printables.com/model/1167162-parametriccustomizable-esp32-box-skadisskadis) | [@adelzu_2519918](https://www.printables.com/@adelzu_2519918) | A flexible and small ESP32 box. |

## Hardware

| Component | Device | Marketplace |
| --- | --- | --- |
| 1x Development board | ESP32 DevKit v1 (USB-C) | [AliExpress](https://es.aliexpress.com/item/1005010020285036.html) |
| 1x 4-pin Press Button | Simple press button | [AliExpress](https://es.aliexpress.com/item/4000600654114.html) |
| 1x AA Battery Holder | AA Battery holder | [AliExpress](https://es.aliexpress.com/item/1005010441704290.html) |

### Complete Wiring Summary

| ESP32 Pin | GPIO Number | Connected To |
|-----------|-------------|--------------|
| **GND** | - | Button Pin-1 |
| **D15** | GPIO 15 | Button Pin-2 |
| **VIN** | - | Battery holder (+) |
| **GND** | - | Battery holder (-) |

## License

Copyright 2026 Grégory Saive <greg@evi.as> for re:Software S.L. (resoftware.es).

Licensed under the [3-Clause BSD License](./LICENSE).
