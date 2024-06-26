add_subdirectory(reader)

# check_function_exists("c8rtomb" FUNCTION_C8RTOMB)

add_executable(muon
  main.c
  name.c
  expr/access.c
  expr/integer.c
  expr/member.c
  expr/record.c
  expr/vector.c
  expr/zero.c
  sign/integer.c
  sign/member.c
  sign/record.c
  sign/vector.c
  stmt/constant.c
  $<TARGET_OBJECTS:reader>
)

target_include_directories(
  muon PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../header")
