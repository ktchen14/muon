find_package(BISON 3.8.0 REQUIRED)
find_package(RE2C 3.0.0 REQUIRED)

BISON_TARGET(SCAN syntax.y "${CMAKE_CURRENT_SOURCE_DIR}/syntax.c")

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/scan.c"
  COMMAND "${RE2C_EXECUTABLE}" ARGS
    -W -Werror --conditions --input-encoding utf8
    --output "${CMAKE_CURRENT_SOURCE_DIR}/scan.c"
    --depfile re2c.d
    "${CMAKE_CURRENT_SOURCE_DIR}/re2c.c"
  COMMENT "Building re2c object source/scan/re2c.c"
  DEPFILE re2c.d VERBATIM
)

set_property(SOURCE "${BISON_SCAN_OUTPUT_SOURCE}" APPEND PROPERTY
  OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/scan.c")

add_library(muon-scan OBJECT "${BISON_SCAN_OUTPUT_SOURCE}")
target_link_libraries(muon-scan PRIVATE muon-common)
