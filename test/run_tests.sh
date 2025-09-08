#!/bin/bash

set -e

TEST_RUNNER="$1"
TEST_FILE="$2"

tmpdir=$(mktemp -d)
trap "rm -rf $tmpdir" EXIT

run_test() {
  [[ -z "$test_name" ]] && return
  echo "Test: $test_name"

  pushd "$tmpdir" > /dev/null
  printf %s "$source" > test.muon
  printf %s "$stdout" > expected_stdout
  printf %s "$stderr" > expected_stderr

  set +e
  ${args:+eval "$TEST_RUNNER $args"} ${args:-"$TEST_RUNNER"} test.muon > stdout 2> stderr
  set -e

  # Strip whitespace from all files
  for i in stdout stderr expected_stdout expected_stderr; do
    sed -i '' -e '/./,$!d' -e :a -e '/^\s*$/N;/\n\s*$/ba' -e 's/\n\s*$//' "$i" 2>/dev/null || true
  done

  diff -u expected_stdout stdout
  diff -u expected_stderr stderr

  popd > /dev/null
}

# Parse test file
while IFS= read -r line; do
  if [[ $line =~ ^Test:\ (.+)$ ]]; then
    run_test
    test_name="${BASH_REMATCH[1]}"
    args= source= stdout= stderr= section=source

  elif [[ $line =~ ^Args:\ (.*)$ ]]; then
    args="${BASH_REMATCH[1]}"

  elif [[ $line =~ ^[[:space:]]+Expected\ Stdout: ]]; then
    section=stdout

  elif [[ $line =~ ^[[:space:]]+Expected\ Stderr: ]]; then
    section=stderr

  elif [[ $line =~ ^[[:space:]]*$ ]] && [[ -n "$section" ]]; then
    declare "$section"="${!section}"$'\n'

  elif [[ -n "$section" ]]; then
    content="${line#  }"  # Strip only the 2-space test indentation
    declare "$section"="${!section}${!section:+$'\n'}$content"
  fi
done < "$TEST_FILE"

run_test
