# GPScan Viewer

[![Build](https://github.com/kojix2/gpscan_viewer/actions/workflows/build.yml/badge.svg)](https://github.com/kojix2/gpscan_viewer/actions/workflows/build.yml)
[![Lines of Code](https://img.shields.io/endpoint?url=https%3A%2F%2Ftokei.kojix2.net%2Fbadge%2Fgithub%2Fkojix2%2Fgpscan_viewer%2Flines)](https://tokei.kojix2.net/github/kojix2/gpscan_viewer)

![Screenshot](https://raw.githubusercontent.com/kojix2/gpscan_viewer/screenshot/root.png)

- Unofficial Qt viewer for [GrandPerspective](https://grandperspectiv.sourceforge.net/) compatible scan XML files
- Works with XML produced by [gpscan](https://github.com/kojix2/gpscan)
- License: GPL (see [COPYING.txt](COPYING.txt))

## Dependencies

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y cmake g++ qt6-base-dev zlib1g-dev
```

### Fedora

```bash
sudo dnf install -y cmake gcc-c++ qt6-qtbase-devel zlib-devel
```

### openSUSE

```bash
sudo zypper install -y cmake gcc-c++ qt6-base-devel zlib-devel
```

### macOS

First, get [GrandPerspective](https://grandperspectiv.sourceforge.net/). That's enough.
If you're still curious about this viewer, install build dependencies via Homebrew:

```bash
brew install cmake qt@6 zlib
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/gpscan_viewer
```

## Test

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## Package (DEB/RPM)

```bash
cmake -S . -B build
cmake --build build
cpack --config build/CPackConfig.cmake
```

Packages are created under `build/` (e.g., `build/*.deb`, `build/*.rpm`).

Update metadata in [CMakeLists.txt](CMakeLists.txt):

- `CPACK_PACKAGE_CONTACT`
- `CPACK_DEBIAN_PACKAGE_MAINTAINER`

## License

This project is released under the GNU General Public License.
See [COPYING.txt](COPYING.txt).

## Acknowledgements

This is an independent, unofficial derivative work of GrandPerspective (macOS) by Erwin Bonsma.
It is not affiliated with or endorsed by Erwin Bonsma or the GrandPerspective project.

Copyright (C) 2005-2025, Erwin Bonsma.

This project was developed primarily with the assistance of AI tools.

GrandPerspective is distributed under the GNU General Public License (GPL), and this project is
distributed under GPL-2.0-or-later accordingly.
