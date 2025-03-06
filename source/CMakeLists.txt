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

  stator/expr.c

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
