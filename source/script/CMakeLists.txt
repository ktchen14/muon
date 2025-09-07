find_package(BISON 3.8.0 REQUIRED)
find_package(RE2C 3.0.0 REQUIRED)

BISON_TARGET(SYNTAX syntax.y syntax.c DEFINES_FILE syntax.h)

add_custom_command(
  OUTPUT re2c.c COMMAND "${RE2C_EXECUTABLE}" ARGS
    "${CMAKE_CURRENT_SOURCE_DIR}/syntax.re2c"
    --output re2c.c
    --depfile re2c.d
    --conditions
    --input-encoding utf8
    -W -Werror
  MAIN_DEPENDENCY syntax.re2c DEPENDS syntax.h
  DEPFILE re2c.d
)

set_property(SOURCE "${BISON_SYNTAX_OUTPUT_SOURCE}" APPEND PROPERTY
  OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/re2c.c")

add_library(syntax OBJECT "${BISON_SYNTAX_OUTPUT_SOURCE}")
target_compile_features(syntax PUBLIC c_std_23)
target_compile_options(syntax PRIVATE
  -iquote "${CMAKE_CURRENT_SOURCE_DIR}"
  -Wall -Wextra -pedantic
  -Wno-gnu-zero-variadic-macro-arguments
  -Wno-gnu-case-range
  -Wno-unused-parameter)
target_include_directories(syntax PRIVATE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/header>"
  "$<INSTALL_INTERFACE:include>")
