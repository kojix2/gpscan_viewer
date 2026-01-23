cmake_minimum_required(VERSION 3.20)

project(gpscan_viewer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)
find_package(ZLIB REQUIRED)

include(GNUInstallDirs)

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

install(TARGETS gpscan_viewer
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES resources/sample.xml
  DESTINATION ${CMAKE_INSTALL_DATADIR}/gpscan_viewer
)

install(FILES resources/gpscan_viewer.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES resources/gpscan_viewer.svg
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

install(FILES COPYING.txt README.md
  DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

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

set(CPACK_PACKAGE_NAME "gpscan_viewer")
set(CPACK_PACKAGE_VENDOR "gpscan_viewer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "maintainer@example.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Viewer for GrandPerspective-compatible scan XML files")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "maintainer@example.com")
set(CPACK_DEBIAN_PACKAGE_RELEASE "1")

# CPack may not have access to dpkg on non-Debian systems.
# Provide a reasonable default architecture mapping.
set(_DEB_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
if(_DEB_ARCH STREQUAL "x86_64")
  set(_DEB_ARCH "amd64")
elseif(_DEB_ARCH STREQUAL "aarch64")
  set(_DEB_ARCH "arm64")
endif()
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${_DEB_ARCH}")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_RPM_PACKAGE_LICENSE "GPL-2.0-or-later")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
include(CPack)
