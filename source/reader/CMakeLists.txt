find_package(BISON REQUIRED)

BISON_TARGET(SYNTAX syntax.y syntax.c DEFINES_FILE syntax.h)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scan.c"
  COMMAND re2c ARGS
    "${CMAKE_CURRENT_SOURCE_DIR}/scan.c"
    --output "${CMAKE_CURRENT_BINARY_DIR}/scan.c"
    --depfile "${CMAKE_CURRENT_BINARY_DIR}/scan.d"
    --api custom
    --case-ranges
    --conditions
    --utf8
    -W -Werror
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/scan.c"
  DEPENDS "${BISON_SYNTAX_OUTPUT_HEADER}"
  DEPFILE "${CMAKE_CURRENT_BINARY_DIR}/scan.d"
)

add_library(
  reader OBJECT
  symbol.c
  "${CMAKE_CURRENT_BINARY_DIR}/scan.c"
  "${BISON_SYNTAX_OUTPUT_SOURCE}")
target_include_directories(reader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../header")
target_include_directories(reader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(reader PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_options(reader PRIVATE
  -Wall -Wextra -Werror -Wno-unused-parameter)
target_compile_features(reader PUBLIC c_std_11)
