add_executable(test_runner test_runner.c)
target_compile_options(test_runner PRIVATE
  -Wall -Wextra -Werror -pedantic
  -Wno-gnu-zero-variadic-macro-arguments)
target_link_libraries(test_runner muon)
target_include_directories(test_runner PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../source"
)

# Add test to build test_runner as a fixture
add_test(
  NAME build_test_runner
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test_runner
)
set_tests_properties(build_test_runner PROPERTIES FIXTURES_SETUP test_runner_fixture)

# Add basic tests with dependency on test runner fixture
add_test(
  NAME basic_tests
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh $<TARGET_FILE:test_runner> ${CMAKE_CURRENT_SOURCE_DIR}/basic_tests.txt
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set_tests_properties(basic_tests PROPERTIES FIXTURES_REQUIRED test_runner_fixture)
