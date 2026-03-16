cmake_minimum_required(VERSION 3.25)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tool")

file(READ VERSION PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)
project(Muon VERSION "${PROJECT_VERSION}"
  DESCRIPTION "An extensible dataflow language"
  HOMEPAGE_URL "https://github.com/ktchen14/muon"
  LANGUAGES C
)

find_package(LLVM REQUIRED CONFIG)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND "${LLVM_DEFINITIONS}")

add_compile_options(
  -Wall -Wextra -pedantic
  -Wcast-align
  -Wcast-qual
  -Wconversion
  -Wno-sign-conversion
  -Wno-logical-op-parentheses
  -Wno-parentheses
  -Wno-unknown-pragmas
  -Wno-unused-parameter
)

add_library(muon-header INTERFACE)
target_compile_features(muon-header INTERFACE c_std_23)

add_subdirectory(header)

add_library(muon SHARED)
target_link_libraries(muon PUBLIC muon-header)
set_property(TARGET muon PROPERTY VERSION "${Muon_VERSION}")

add_executable(muon-main source/main.c)
set_target_properties(muon-main PROPERTIES OUTPUT_NAME muon)
target_compile_definitions(muon-main PRIVATE ${LLVM_DEFINITIONS_LIST})
target_include_directories(muon-main PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(muon-main PRIVATE muon)

add_subdirectory(source)

add_subdirectory(ruby)

file(GLOB_RECURSE FORMAT_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/header/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h"
)
add_custom_target(format COMMAND clang-format -i ${FORMAT_SOURCES} VERBATIM)

if(PROJECT_IS_TOP_LEVEL)
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
