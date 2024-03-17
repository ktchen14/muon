find_package(BISON REQUIRED)

BISON_TARGET(SYNTAX syntax.y syntax.c DEFINES_FILE syntax.h)

add_executable(main
  main.c
  "${BISON_SYNTAX_OUTPUT_SOURCE}")
