add_subdirectory(script)

target_sources(muon PRIVATE
  common.c

  author/author.c
  author/coercion.c
  author/node.c
  author/type.c

  inductor/coercion.c
  inductor/core.c
  inductor/detect.c
  inductor/induce.c
  inductor/reduce.c
  inductor/type.c
  inductor/universe.c
  inductor/node.c

  runner.c
  script.c
  standard.c
  status.c

  stator/name.c
  stator/node.c

  $<TARGET_OBJECTS:reader>
)

target_sources(main PRIVATE main.c)
