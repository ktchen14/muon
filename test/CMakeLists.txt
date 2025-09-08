add_executable(main-test EXCLUDE_FROM_ALL main.c)
set_target_properties(main-test PROPERTIES OUTPUT_NAME muon)
target_compile_options(main-test PRIVATE
  -Wall -Wextra -Werror -pedantic
  -Wno-gnu-zero-variadic-macro-arguments)
target_link_libraries(main-test muon)

add_test(NAME main-test
  COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target main-test
)
set_tests_properties(main-test PROPERTIES FIXTURES_SETUP main-test)

add_test(NAME basic_tests.txt
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh" $<TARGET_FILE:main-test> "${CMAKE_CURRENT_SOURCE_DIR}/basic_tests.txt"
)
set_tests_properties(basic_tests.txt PROPERTIES FIXTURES_REQUIRED main-test)
