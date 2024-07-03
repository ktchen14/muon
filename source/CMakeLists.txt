add_subdirectory(script)

# check_function_exists("c8rtomb" FUNCTION_C8RTOMB)

add_library(muon)

target_sources(muon PRIVATE
  analyzer.c
  common.c
  inductor.c
  main.c
  name.c
  script.c
  # status.c

  node.c
  node/access_expr.c
  node/constant_stmt.c
  node/integer_expr.c
  node/integer_sign.c
  node/member_expr.c
  node/member_sign.c
  node/name_expr.c
  node/name_sign.c
  node/record_expr.c
  node/record_sign.c
  node/type_stmt.c
  node/variable_sign.c
  node/vector_expr.c
  node/vector_sign.c
  node/zero_expr.c

  type.c
  type/integer.c
  type/variable.c
  type/vector.c

  $<TARGET_OBJECTS:reader>
)

target_include_directories(
  muon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../header")
set_property(TARGET muon PROPERTY VERSION "${Muon_VERSION}")

add_executable(muon-main main.c)
target_link_libraries(muon-main PRIVATE muon)
set_property(TARGET muon-main PROPERTY VERSION "${Muon_VERSION}")
