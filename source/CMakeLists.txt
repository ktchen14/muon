add_subdirectory(scan)

target_sources(muon PRIVATE
  common.c

  author/author.c
  author/coercion.c
  author/node.c
  author/type.c

  engine/name.c
  engine/node.c

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

  $<TARGET_OBJECTS:syntax>
)
