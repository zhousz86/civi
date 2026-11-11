# civi_platform — display-independent services shared by every HMI process.
# Depends on Qt Core only, so it can also back non-Quick consumers (e.g. a Kanzi or
# Android Automotive front-end through a thin binding) — CIVI-SW-008.
add_library(civi_platform STATIC
  include/civi/PpsChannel.h          src/PpsChannel.cpp
  include/civi/CiviConfig.h          src/CiviConfig.cpp
  include/civi/VehicleServices.h     src/VehicleServices.cpp
  include/civi/DistractionPolicy.h   src/DistractionPolicy.cpp
  include/civi/HmiSettings.h         src/HmiSettings.cpp
  include/civi/CatalogTranslator.h   src/CatalogTranslator.cpp
  include/civi/AlertArbiter.h        src/AlertArbiter.cpp
  include/civi/ConnectivityModel.h   src/ConnectivityModel.cpp
  include/civi/EmergencyService.h    src/EmergencyService.cpp
  include/civi/ConsentManager.h      src/ConsentManager.cpp
  include/civi/OtaService.h          src/OtaService.cpp
  include/civi/NavigationModel.h     src/NavigationModel.cpp
  include/civi/MediaModel.h          src/MediaModel.cpp
)
target_include_directories(civi_platform PUBLIC include)
target_link_libraries(civi_platform PUBLIC Qt6::Core)
target_compile_definitions(civi_platform PUBLIC CIVI_VERSION="${PROJECT_VERSION}")
if(CMAKE_SYSTEM_NAME STREQUAL "QNX")
  target_compile_definitions(civi_platform PUBLIC CIVI_TARGET_QNX=1)
endif()
