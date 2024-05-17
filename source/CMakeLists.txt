add_subdirectory(reader)

add_executable(
  muon main.c name.c $<TARGET_OBJECTS:reader>
)

target_include_directories(
  muon PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../header")
