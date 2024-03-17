find_program(RE2C_EXECUTABLE NAMES re2c DOC "path to the re2c executable")
mark_as_advanced(RE2C_EXECUTABLE)

if(RE2C_EXECUTABLE)
  execute_process(
    COMMAND "${RE2C_EXECUTABLE}" --version
    OUTPUT_VARIABLE RE2C_version_output
    ERROR_VARIABLE RE2C_version_error
    RESULT_VARIABLE RE2C_version_result
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT ${RE2C_version_result} EQUAL 0)
    message(SEND_ERROR "Command \"${RE2C_EXECUTABLE} --version\" failed with output:\n${RE2C_version_error}")
  else()
    string(REGEX REPLACE "^re2c (.*)$" "\\1" RE2C_VERSION "${RE2C_version_output}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RE2C REQUIRED_VARS RE2C_EXECUTABLE VERSION_VAR RE2C_VERSION)
