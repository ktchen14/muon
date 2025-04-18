find_package(LLVM REQUIRED CONFIG)

add_library(emitter OBJECT node.c type.c)
target_compile_features(emitter PUBLIC c_std_23)
target_compile_options(emitter PRIVATE
  -Wall -Wextra -pedantic
  -Wconversion
  -Wno-gnu-zero-variadic-macro-arguments  # Ignore , ##__VA_ARGS__
  -Wno-logical-op-parentheses
  -Wno-parentheses
  -Wno-unknown-pragmas
  -Wno-unused-parameter
  -Wcast-align
)
target_include_directories(emitter PRIVATE ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(emitter PRIVATE ${LLVM_DEFINITIONS_LIST})

target_include_directories(emitter PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../header>"
  "$<INSTALL_INTERFACE:include>")

llvm_map_components_to_libnames(llvm_libs mcjit interpreter native)
target_link_libraries(emitter ${llvm_libs} stdc++)
