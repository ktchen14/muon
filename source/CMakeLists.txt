add_subdirectory(script)

target_sources(muon PRIVATE
  analyzer.c
  common.c
  inductor.c
  main.c
  name.c
  node.c
  script.c
  type.c

  node/access_expr.c
  node/integer_expr.c
  node/member_expr.c
  node/name_expr.c
  node/record_expr.c
  node/vector_expr.c
  node/zero_expr.c

  node/integer_sign.c
  node/member_sign.c
  node/name_sign.c
  node/record_sign.c
  node/variable_sign.c
  node/vector_sign.c

  node/constant_stmt.c
  node/type_stmt.c

  type/integer.c
  type/variable.c
  type/vector.c

  $<TARGET_OBJECTS:reader>
)

target_sources(main PRIVATE main.c)
