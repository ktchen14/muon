find_package(BISON 3.8.0 REQUIRED)
find_package(RE2C 3.0.0 REQUIRED)

BISON_TARGET(SYNTAX syntax.y syntax.c DEFINES_FILE syntax.h)
set_property(SOURCE "${BISON_SYNTAX_OUTPUT_SOURCE}"
  APPEND PROPERTY OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/re2c.c")

add_custom_command(
  OUTPUT re2c.c
  COMMAND "${RE2C_EXECUTABLE}" ARGS
    "${CMAKE_CURRENT_SOURCE_DIR}/syntax.re2c"
    --output re2c.c
    --depfile re2c.d
    --conditions
    --input-encoding utf8
    -W -Werror
  MAIN_DEPENDENCY syntax.re2c
  DEPENDS syntax.h
  DEPFILE re2c.d
)

add_library(reader OBJECT "${BISON_SYNTAX_OUTPUT_SOURCE}")
target_include_directories(reader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../header")
target_include_directories(reader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(reader PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_features(reader PUBLIC c_std_11)
