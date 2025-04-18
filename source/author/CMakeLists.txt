find_package(LLVM REQUIRED CONFIG)

add_library(author OBJECT node.c type.c)
target_compile_features(author PUBLIC c_std_23)
target_compile_options(author PRIVATE
  -Wall -Wextra -pedantic
  -Wconversion
  -Wno-gnu-zero-variadic-macro-arguments  # Ignore , ##__VA_ARGS__
  -Wno-logical-op-parentheses
  -Wno-parentheses
  -Wno-unknown-pragmas
  -Wno-unused-parameter
  -Wcast-align
)
target_include_directories(author PRIVATE ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(author PRIVATE ${LLVM_DEFINITIONS_LIST})

target_include_directories(author PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../header>"
  "$<INSTALL_INTERFACE:include>")

llvm_map_components_to_libnames(llvm_libs mcjit interpreter native)
target_link_libraries(author ${llvm_libs} stdc++)
