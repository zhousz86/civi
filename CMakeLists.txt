# CIVI cockpit front-end — top-level build
# Host (Linux desktop simulation) and target (QNX SDP 8.0, aarch64le) from one tree.
#   Host:   cmake -B build -S . && cmake --build build
#   Target: cmake -B build-qnx -S . -DCMAKE_TOOLCHAIN_FILE=cmake/qnx-sdp8-aarch64le.cmake \
#                 -DCMAKE_PREFIX_PATH=<Qt-for-QNX install>/lib/cmake
cmake_minimum_required(VERSION 3.21)
project(civi-frontend VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CIVI_BUILD_TESTS     "Build unit tests"                         ON)
option(CIVI_BUILD_HMI       "Build Qt Quick display applications"      ON)
option(CIVI_BUILD_TELLTALED "Build the safety telltale renderer"       ON)

# Values the OEM still owns (see config/civi-config.json and requirements §12.2)
# are runtime configuration, never compile-time constants (CIVI-DAT-009).

if(CIVI_BUILD_TELLTALED)
  add_subdirectory(safety/civi-telltaled)
endif()

set(CMAKE_AUTOMOC ON)
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Qml Quick)
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.5)
  qt_standard_project_setup(REQUIRES 6.4)
else()
  qt_standard_project_setup()
endif()

add_subdirectory(platform)
if(CIVI_BUILD_HMI)
  add_subdirectory(hmi)
endif()

if(CIVI_BUILD_TESTS)
  enable_testing()
  find_package(Qt6 REQUIRED COMPONENTS Test)
  add_subdirectory(tests)
endif()

install(FILES config/civi-config.json config/policy/consent-policy.json
        DESTINATION etc/civi)
