add_subdirectory(reader)

# check_function_exists("c8rtomb" FUNCTION_C8RTOMB)

add_executable(muon
  main.c
  name.c
  expr/integer.c
  expr/vector.c
  stmt/constant.c
  $<TARGET_OBJECTS:reader>
)

target_include_directories(
  muon PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../header")
