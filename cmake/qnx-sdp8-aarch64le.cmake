# CMake toolchain file: QNX SDP 8.0, 64-bit Arm (Qualcomm SA8295P / SA8775P / SA8255P class SoCs)
# Prerequisite: source <SDP>/qnxsdp-env.sh so QNX_HOST and QNX_TARGET are set.
if(NOT DEFINED ENV{QNX_HOST} OR NOT DEFINED ENV{QNX_TARGET})
  message(FATAL_ERROR "QNX_HOST/QNX_TARGET not set — source qnxsdp-env.sh from QNX SDP 8.0 first")
endif()

set(CMAKE_SYSTEM_NAME QNX)
set(CMAKE_SYSTEM_VERSION 8.0.0)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CIVI_QNX_ARCH gcc_ntoaarch64le)
set(CMAKE_C_COMPILER   $ENV{QNX_HOST}/usr/bin/qcc)
set(CMAKE_CXX_COMPILER $ENV{QNX_HOST}/usr/bin/q++)
set(CMAKE_C_COMPILER_TARGET   ${CIVI_QNX_ARCH})
set(CMAKE_CXX_COMPILER_TARGET ${CIVI_QNX_ARCH})

set(CMAKE_SYSROOT $ENV{QNX_TARGET})
set(CMAKE_FIND_ROOT_PATH $ENV{QNX_TARGET} $ENV{QNX_TARGET}/aarch64le)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)   # Qt-for-QNX lives outside the sysroot

# Hardening flags consistent with ISO/SAE 21434 secure-coding expectations (CIVI-SEC-007)
set(CMAKE_C_FLAGS_INIT   "-fstack-protector-strong -D_FORTIFY_SOURCE=2")
set(CMAKE_CXX_FLAGS_INIT "-fstack-protector-strong -D_FORTIFY_SOURCE=2")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-z,relro,-z,now")
