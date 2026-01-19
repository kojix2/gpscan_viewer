cmake_minimum_required(VERSION 3.20)

project(gpscan_viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)
find_package(ZLIB REQUIRED)

enable_testing()

add_executable(gpscan_viewer
  src/main.cpp
  src/ViewerWindow.cpp
  src/CanvasWidget.cpp
  src/TreeModel.cpp
  src/TreeReader.cpp
  src/TreeLayout.cpp
  src/Utils.cpp
)

target_link_libraries(gpscan_viewer PRIVATE Qt6::Widgets ZLIB::ZLIB)

add_executable(gpscan_viewer_tests
  tests/TestMain.cpp
  src/TreeLayout.cpp
  src/TreeModel.cpp
  src/TreeReader.cpp
  src/Utils.cpp
)

target_include_directories(gpscan_viewer_tests PRIVATE src)
target_link_libraries(gpscan_viewer_tests PRIVATE Qt6::Core ZLIB::ZLIB)

add_test(NAME gpscan_viewer_tests COMMAND gpscan_viewer_tests)
