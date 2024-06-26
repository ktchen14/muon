add_subdirectory(reader)

# check_function_exists("c8rtomb" FUNCTION_C8RTOMB)

add_library(muon)

target_sources(muon PRIVATE
  common.c
  main.c
  name.c
  script.c
  # status.c

  expr.c
  expr/access.c
  expr/integer.c
  expr/member.c
  expr/name.c
  expr/record.c
  expr/vector.c
  expr/zero.c

  sign.c
  sign/integer.c
  sign/member.c
  sign/name.c
  sign/record.c
  sign/vector.c

  stmt.c
  stmt/constant.c

  $<TARGET_OBJECTS:reader>
)

target_include_directories(
  muon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../header")
set_property(TARGET muon PROPERTY VERSION "${Muon_VERSION}")

add_executable(muon-main main.c)
target_link_libraries(muon-main PRIVATE muon)
set_property(TARGET muon-main PROPERTY VERSION "${Muon_VERSION}")
