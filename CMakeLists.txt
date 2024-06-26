cmake_minimum_required(VERSION 3.25)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tool")

file(READ VERSION PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)
project("Muon" VERSION "${PROJECT_VERSION}"
  DESCRIPTION "An extensible dataflow language"
  HOMEPAGE_URL "https://github.com/ktchen14/muon"
  LANGUAGES C
)
set(CMAKE_C_FLAGS "-g")

add_compile_options(-Werror -Wall -Wextra -Wno-unused-parameter)

add_subdirectory(source)

if(PROJECT_IS_TOP_LEVEL)
  set(BUILD_TESTING ON CACHE BOOL "Build the testing tree.")
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
