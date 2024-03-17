find_package(Ruby COMPONENTS Development)
if(NOT Ruby_FOUND)
  return()
endif()

add_library(muon-ruby MODULE EXCLUDE_FROM_ALL
  common.c common.h muon.c node.c node.h)
get_target_property(suffix Ruby::Module INTERFACE_RUBY_EXTENSION_SUFFIX)
set_target_properties(muon-ruby PROPERTIES
  OUTPUT_NAME muon PREFIX "" SUFFIX "${suffix}"
)
target_link_libraries(muon-ruby PRIVATE muon Ruby::Module)

# Strip sanitizer flags — they can't be loaded into the Ruby process
target_compile_options(muon-ruby PRIVATE -fno-sanitize=all)
target_link_options(muon-ruby PRIVATE -fno-sanitize=all)
