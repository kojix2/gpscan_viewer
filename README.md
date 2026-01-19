# gpscan_viewer

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