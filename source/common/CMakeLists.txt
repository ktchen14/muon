add_library(muon-common OBJECT
  common.h
  debug.c debug.h
  hash.c hash.h
)
target_link_libraries(muon-common PRIVATE muon-header)
