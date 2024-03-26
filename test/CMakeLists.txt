add_executable(
  scan_main
  scan_main.c
  $<TARGET_OBJECTS:reader>
)
target_include_directories(
  scan_main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../source")
