# Test configuration for Muon language tests

# Enable testing

# Build test runner executable
add_executable(test_runner test_runner.c)

# Link against the muon library and dependencies
target_link_libraries(test_runner
    muon
)

# Include directories
target_include_directories(test_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../header
    ${CMAKE_CURRENT_SOURCE_DIR}/../source
)

# Copy test files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/basic_tests.txt
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                      GROUP_READ GROUP_EXECUTE
                      WORLD_READ WORLD_EXECUTE)

# Add basic tests
add_test(
    NAME basic_tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh $<TARGET_FILE:test_runner> ${CMAKE_CURRENT_BINARY_DIR}/basic_tests.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
