# add_executable(scan_main
#   scan_main.c
#   ../source/node/integer_expr.c
#   ../source/name.c
#   $<TARGET_OBJECTS:reader>
# )

# target_link_libraries(scan_main PRIVATE muon)
# target_include_directories(
#   scan_main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../source")
# target_include_directories(
#   scan_main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../header")
