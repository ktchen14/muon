cmake_minimum_required(VERSION 3.25)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tool")

file(READ VERSION PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)
project(Muon VERSION "${PROJECT_VERSION}"
  DESCRIPTION "An extensible dataflow language"
  HOMEPAGE_URL "https://github.com/ktchen14/muon"
  LANGUAGES C
)

add_library(muon SHARED)
target_compile_features(muon PUBLIC c_std_23)
target_compile_options(muon PRIVATE
  -Wall -Wextra -pedantic
  -Wconversion
  -Wno-gnu-zero-variadic-macro-arguments  # Ignore , ##__VA_ARGS__
  -Wno-logical-op-parentheses
  -Wno-parentheses
  -Wno-unknown-pragmas
  -Wno-unused-parameter
  -Wcast-align
)
target_include_directories(muon PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/header>"
  "$<INSTALL_INTERFACE:include>")
set_property(TARGET muon PROPERTY VERSION "${Muon_VERSION}")

add_executable(main)
target_compile_options(main PRIVATE
  -Wall -Wextra -pedantic
  -Wconversion
  -Wno-gnu-zero-variadic-macro-arguments  # Ignore , ##__VA_ARGS__
  -Wno-logical-op-parentheses
  -Wno-parentheses
  -Wno-unknown-pragmas
  -Wno-unused-parameter
  -Wcast-align
)
target_link_libraries(main PRIVATE muon)

find_package(LLVM REQUIRED CONFIG)
target_include_directories(main PRIVATE ${LLVM_INCLUDE_DIRS})
target_include_directories(muon PRIVATE ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(main PRIVATE ${LLVM_DEFINITIONS_LIST})
target_compile_definitions(muon PRIVATE ${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(llvm_libs mcjit interpreter native orcjit)
target_link_libraries(muon PRIVATE ${llvm_libs} stdc++)
target_link_libraries(main PRIVATE ${llvm_libs} stdc++)

add_subdirectory(source)

if(PROJECT_IS_TOP_LEVEL)
  set(BUILD_TESTING ON CACHE BOOL "Build the testing tree.")
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
