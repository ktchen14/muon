cmake_minimum_required(VERSION 3.25)

project("Muon"
  VERSION "${PROJECT_VERSION}"
  DESCRIPTION "A hermetic configuration language"
  HOMEPAGE_URL "https://github.com/ktchen14/muon"
  LANGUAGES C
)

add_subdirectory(source)
add_subdirectory(test)
