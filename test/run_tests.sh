#!/bin/bash -e

[ $# -eq 2 ] || { echo "Usage: $0 MUON TEST" 1>&2; exit 1; }

MUON="$1"
exec < "$2"

mkdir -p test

run_test() {
  [[ -z "$test_name" ]] && return
  echo "Test: $test_name"

  mkdir -p "test/$test_name"
  output="test/$test_name"

  printf %s "$source" > "$output/test.muon"
  printf %s "$stdout" > "$output/expected_stdout"
  printf %s "$stderr" > "$output/expected_stderr"

  set +e
  ${args:+eval "$MUON $args"} ${args:-"$MUON"} "$output/test.muon" > "$output/stdout" 2> "$output/stderr"
  set -e

  # Strip whitespace from all files
  for i in stdout stderr expected_stdout expected_stderr; do
    sed -i '' -e '/./,$!d' -e :a -e '/^\s*$/N;/\n\s*$/ba' -e 's/\n\s*$//' "$output/$i" 2>/dev/null || true
  done

  diff -u "$output"/{expected_,}stdout
  diff -u "$output"/{expected_,}stderr
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
done

run_test
