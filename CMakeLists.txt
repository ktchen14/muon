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
target_compile_features(muon INTERFACE c_std_11)
target_compile_features(muon PRIVATE c_std_23)
target_compile_options(muon PRIVATE
  -Wall -Wextra -pedantic
  -Wcast-align
  -Wconversion
  -Wno-gnu-case-range
  -Wno-gnu-zero-variadic-macro-arguments  # Ignore , ##__VA_ARGS__
  -Wno-logical-op-parentheses
  -Wno-parentheses
  -Wno-unknown-pragmas
  -Wno-unused-parameter
)
target_include_directories(muon PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/header>"
  "$<INSTALL_INTERFACE:include>")
set_property(TARGET muon PROPERTY VERSION "${Muon_VERSION}")

# LLVM
find_package(LLVM REQUIRED CONFIG)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND "${LLVM_DEFINITIONS}")
target_compile_definitions(muon PRIVATE ${LLVM_DEFINITIONS_LIST})
target_include_directories(muon PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(muon PRIVATE LLVM LLVM-C)

add_executable(main source/main.c)
set_target_properties(main PROPERTIES OUTPUT_NAME muon)
target_compile_options(main PRIVATE
  -Wall -Wextra -pedantic
  -Wconversion
  -Wno-gnu-case-range
  -Wno-gnu-zero-variadic-macro-arguments  # Ignore , ##__VA_ARGS__
  -Wno-logical-op-parentheses
  -Wno-parentheses
  -Wno-unknown-pragmas
  -Wno-unused-parameter
  -Wcast-align
)
target_compile_definitions(main PRIVATE ${LLVM_DEFINITIONS_LIST})
target_compile_features(main PRIVATE c_std_23)
target_include_directories(main PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(main PRIVATE LLVM LLVM-C)
target_link_libraries(main PRIVATE muon)

add_subdirectory(source)

file(GLOB_RECURSE FORMAT_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/header/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h"
)
add_custom_target(format COMMAND clang-format -i ${FORMAT_SOURCES} VERBATIM)

# Ruby extension
find_package(Ruby COMPONENTS Development)
if(Ruby_FOUND)
  add_library(ruby MODULE ruby/muon.c)
  get_target_property(suffix Ruby::Module INTERFACE_RUBY_EXTENSION_SUFFIX)
  set_target_properties(ruby PROPERTIES
    OUTPUT_NAME muon PREFIX "" SUFFIX "${suffix}"
  )
  target_link_libraries(ruby PRIVATE muon Ruby::Module)

  # Strip sanitizer flags — they can't be loaded into the Ruby process
  target_compile_options(ruby PRIVATE -fno-sanitize=all)
  target_link_options(ruby PRIVATE -fno-sanitize=all)
endif()

if(PROJECT_IS_TOP_LEVEL)
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
