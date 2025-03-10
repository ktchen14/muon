add_subdirectory(script)

target_sources(muon PRIVATE
  inductor/coercion.c
  inductor/detect.c
  inductor/induce.c
  inductor/reduce.c
  inductor/type.c

  # inductor/reduce.c
  script.c
  status.c

  stator/debug.c
  stator/name.c
  stator/node.c

  stator/expr.c
  stator/sign.c
  stator/stmt.c
  stator/view.c

  $<TARGET_OBJECTS:reader>
)

target_sources(main PRIVATE main.c)
