add_library(muon-author OBJECT
  author.c
  coercion.c
  node.c
  type.c
)
target_link_libraries(muon-author PRIVATE muon-common)

target_compile_definitions(muon-author PRIVATE ${LLVM_DEFINITIONS_LIST})
target_include_directories(muon-author PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(muon-author PUBLIC LLVM LLVM-C)
