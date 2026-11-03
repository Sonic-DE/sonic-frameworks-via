<!-- SPDX-License-Identifier: BSD-2-Clause -->
<!-- SPDX-FileCopyrightText: SonicDE -->

# SonicVia

A standalone C++17/20 library implementing the QMK [VIA](https://www.caniusevia.com/) HID
keyboard protocol for RGB lighting control.

## Features

- Pure enumeration of VIA keyboards (HID usage page `0xff60`) via `ViaDiscovery`.
- Single-device blocking HID access via `ViaDevice` (hidraw backend on Linux).
- Protocol-version-aware command encoding (VIA V3 ≥ 11 + legacy < 11).
- Single-byte HSV color conversion from `QColor`.
- `applyColor(QColor)` sets the color through RGB Matrix, falling back to RGB Light.
- `hasLightingSupport()` tells whether either lighting channel responds.
- Stub build when hidapi is unavailable (`HAVE_VIA_SUPPORT` undefined).

## Requirements

- Qt 6.10.0 or newer (Core, Gui)
- KDE ECM 6.26.0 or newer
- CMake 3.21 or newer
- A C++20 compiler
- `libhidapi-hidraw` (or `hidapi-hidraw`/`hidapi` pkg-config module) at build time

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

To build without hidapi (stub mode):

```sh
cmake -S . -B build-stub -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_DISABLE_FIND_PACKAGE_PkgConfig=ON
cmake --build build-stub
```

## Install

```sh
cmake --install build --prefix /usr/local
```

## Consume

```cmake
find_package(SonicVia 6.27.0 REQUIRED)
target_link_libraries(myapp PRIVATE SonicVia::SonicVia)
```

## License

Licensed under the GNU Lesser General Public License 2.1 or later, the GNU Lesser General
Public License 3.0 only, or the KDE-Accepted LGPL. See [LICENSES/](LICENSES/) for the full
license texts.
