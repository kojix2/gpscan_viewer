Dependencies (Linux)

Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y cmake g++ qt6-base-dev zlib1g-dev
```

Fedora

```bash
sudo dnf install -y cmake gcc-c++ qt6-qtbase-devel zlib-devel
```

openSUSE

```bash
sudo zypper install -y cmake gcc-c++ qt6-base-devel zlib-devel
```

Build

```bash
cmake -S . -B build
cmake --build build
```

Run

```bash
./build/gpscan_viewer
```

Test

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

Package (DEB/RPM)

```bash
cmake -S . -B build
cmake --build build
cpack --config build/CPackConfig.cmake
```

Packages are created under build (e.g., build/*.deb, build/*.rpm).

Update metadata in [CMakeLists.txt](CMakeLists.txt):

- `CPACK_PACKAGE_CONTACT`
- `CPACK_DEBIAN_PACKAGE_MAINTAINER`