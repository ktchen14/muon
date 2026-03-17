# add_subdirectory(author)
# target_link_libraries(muon PUBLIC muon-author)

add_subdirectory(engine)
target_link_libraries(muon PUBLIC muon-engine)

add_subdirectory(scan)
target_link_libraries(muon PUBLIC muon-scan)

target_sources(muon PRIVATE
  common.c
  hash.c hash.h

  inductor/common.c
  inductor/induce.c
  inductor/node.c
  inductor/reduce.c
  inductor/scheme.c

  detector/detect.c

  # reductor/coercion.c
  # reductor/reduce.c

  # runner.c
  standard.c standard.h
  status.c
)
