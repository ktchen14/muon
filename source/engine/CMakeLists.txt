add_executable(muon-engine-generate generate.c)
target_link_libraries(muon-engine-generate PRIVATE muon-common)

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/header/muon/engine")
add_custom_command(
  OUTPUT "${PROJECT_BINARY_DIR}/header/muon/engine/object.h"
  COMMAND
    muon-engine-generate "${PROJECT_BINARY_DIR}/header/muon/engine/object.h"
  DEPENDS muon-engine-generate
)

add_library(muon-engine OBJECT
  "${PROJECT_BINARY_DIR}/header/muon/engine/object.h"
  common.h common.c
  core.c core.h
  module.c module.h
  name.c name.h
  node.c node.h
  type.c type.h
)
target_link_libraries(muon-engine PRIVATE muon-common)
target_sources(muon-engine INTERFACE FILE_SET HEADERS
  BASE_DIRS "${PROJECT_BINARY_DIR}/header"
  FILES "${PROJECT_BINARY_DIR}/header/muon/engine/object.h"
)
