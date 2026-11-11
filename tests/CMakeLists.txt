qt_add_executable(tst_platform tst_platform.cpp)
target_link_libraries(tst_platform PRIVATE civi_platform Qt6::Test)
target_compile_definitions(tst_platform PRIVATE
  CIVI_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
add_test(NAME platform COMMAND tst_platform)
set_tests_properties(platform PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME french_parity
           COMMAND ${Python3_EXECUTABLE} -I ${PROJECT_SOURCE_DIR}/tools/i18n/check_parity.py --root ${PROJECT_SOURCE_DIR})
  add_test(NAME sbom_provenance
           COMMAND ${Python3_EXECUTABLE} -I ${PROJECT_SOURCE_DIR}/tools/sbom/generate_sbom.py
                   --root ${PROJECT_SOURCE_DIR} --out ${CMAKE_BINARY_DIR}/civi-sbom.cdx.json --market US)
endif()
