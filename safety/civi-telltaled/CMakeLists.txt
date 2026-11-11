# civi-telltaled — minimal C telltale renderer for the cluster partition.
# Deliberately independent of Qt, the GPU and the infotainment stack: it writes pixels
# into a Screen window buffer that the display controller composites on its own
# pipeline/z-order, so telltales survive a crash, overload or update of any HMI process
# (CIVI-PLT-001, -PLT-002, -HMI-008). Started from the IFS before the HMI (CIVI-PLT-003).
add_executable(civi-telltaled main.c telltale.c telltale.h render.h)
set_property(TARGET civi-telltaled PROPERTY C_STANDARD 99)
if(CMAKE_SYSTEM_NAME STREQUAL "QNX")
  target_sources(civi-telltaled PRIVATE render_screen.c)
  target_link_libraries(civi-telltaled PRIVATE screen)
  target_compile_definitions(civi-telltaled PRIVATE CIVI_TARGET_QNX=1)
else()
  target_sources(civi-telltaled PRIVATE render_host.c)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|QCC")
  target_compile_options(civi-telltaled PRIVATE -Wall -Wextra -Wconversion -Werror)
endif()
install(TARGETS civi-telltaled RUNTIME DESTINATION bin)

if(CIVI_BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING)
  add_executable(tst_telltale tst_telltale.c telltale.c)
  set_property(TARGET tst_telltale PROPERTY C_STANDARD 99)
  enable_testing()
  add_test(NAME telltale_logic COMMAND tst_telltale)
endif()
