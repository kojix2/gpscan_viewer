# gpscan_viewer

## Dependencies (Linux)

This project uses Qt6 and zlib. Install the packages below for your distro.

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

CPack is configured to generate both DEB and RPM packages.

```bash
cmake -S . -B build
cmake --build build
cpack --config build/CPackConfig.cmake
```

The generated packages will be placed under the build directory (e.g., build/*.deb, build/*.rpm).

### Customize maintainer metadata

Update these fields in [CMakeLists.txt](CMakeLists.txt):

- `CPACK_PACKAGE_CONTACT`
- `CPACK_DEBIAN_PACKAGE_MAINTAINER`