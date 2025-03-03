add_subdirectory(script)

target_sources(muon PRIVATE
  inductor/detect.c
  inductor/induce.c
  inductor/type.c

  # inductor/reduce.c
  script.c
  status.c

  stator/debug.c
  stator/name.c
  stator/node.c

  stator/access_expr.c
  stator/boolean_expr.c
  stator/coerce_expr.c
  stator/integer_expr.c
  stator/invoke_expr.c
  stator/lambda_expr.c
  stator/name_expr.c
  stator/record_expr.c
  stator/sequence_expr.c
  stator/vector_expr.c
  stator/zero_expr.c

  stator/boolean_sign.c
  stator/integer_sign.c
  stator/name_sign.c
  stator/record_sign.c
  stator/variable_sign.c
  stator/vector_sign.c

  stator/define_stmt.c
  stator/type_stmt.c

  stator/variable_view.c

  $<TARGET_OBJECTS:reader>
)

target_sources(main PRIVATE main.c)
