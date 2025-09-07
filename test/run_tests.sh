#!/bin/bash

# Test runner for Muon language tests
# Usage: run_tests.sh <test_runner_executable> <test_file>

set -e

TEST_RUNNER="$1"
TEST_FILE="$2"

# Parse the test file
test_name=""
args=""
input_file=""
expected_stdout=""
expected_stderr=""
current_section=""

# Create temporary files
temp_dir=$(mktemp -d)
input_temp="$temp_dir/input.muon"
stdout_temp="$temp_dir/stdout"
stderr_temp="$temp_dir/stderr"
expected_stdout_temp="$temp_dir/expected_stdout"
expected_stderr_temp="$temp_dir/expected_stderr"

cleanup() {
    rm -rf "$temp_dir"
}
trap cleanup EXIT

# Function to run a single test
run_test() {
    if [ -z "$test_name" ]; then
        return
    fi
    
    echo "Running test: $test_name"
    
    # Write input to temp file
    echo -n "$input_file" > "$input_temp"
    
    # Write expected outputs to temp files
    echo -n "$expected_stdout" > "$expected_stdout_temp"
    echo -n "$expected_stderr" > "$expected_stderr_temp"
    
    # Run the test
    set +e
    if [ -n "$args" ]; then
        eval "$TEST_RUNNER $args \"$input_temp\"" > "$stdout_temp" 2> "$stderr_temp"
    else
        "$TEST_RUNNER" "$input_temp" > "$stdout_temp" 2> "$stderr_temp"
    fi
    exit_code=$?
    set -e
    
    # Strip leading and trailing blank lines from outputs
    sed -i '' -e '/./,$!d' -e :a -e '/^\s*$/N;/\n\s*$/ba' -e 's/\n\s*$//' "$expected_stdout_temp" || true
    sed -i '' -e '/./,$!d' -e :a -e '/^\s*$/N;/\n\s*$/ba' -e 's/\n\s*$//' "$stdout_temp" || true
    sed -i '' -e '/./,$!d' -e :a -e '/^\s*$/N;/\n\s*$/ba' -e 's/\n\s*$//' "$expected_stderr_temp" || true
    sed -i '' -e '/./,$!d' -e :a -e '/^\s*$/N;/\n\s*$/ba' -e 's/\n\s*$//' "$stderr_temp" || true
    
    # Compare outputs
    if ! diff -q "$expected_stdout_temp" "$stdout_temp" > /dev/null 2>&1; then
        echo "FAIL: $test_name - stdout mismatch"
        echo "Expected stdout:"
        cat "$expected_stdout_temp"
        echo "Actual stdout:"
        cat "$stdout_temp"
        return 1
    fi
    
    if ! diff -q "$expected_stderr_temp" "$stderr_temp" > /dev/null 2>&1; then
        echo "FAIL: $test_name - stderr mismatch"
        echo "Expected stderr:"
        cat "$expected_stderr_temp"
        echo "Actual stderr:"
        cat "$stderr_temp"
        return 1
    fi
    
    echo "PASS: $test_name"
    return 0
}

# Parse the test file
while IFS= read -r line; do
    if [[ $line =~ ^Test:\ (.+)$ ]]; then
        # Run previous test if exists
        run_test
        
        # Start new test
        test_name="${BASH_REMATCH[1]}"
        args=""
        input_file=""
        expected_stdout=""
        expected_stderr=""
        current_section="input"
    elif [[ $line =~ ^Args:\ (.*)$ ]]; then
        args="${BASH_REMATCH[1]}"
    elif [[ $line =~ ^[[:space:]]+Expected\ Stdout: ]]; then
        current_section="expected_stdout"
    elif [[ $line =~ ^[[:space:]]+Expected\ Stderr: ]]; then
        current_section="expected_stderr"
    elif [[ $line =~ ^[[:space:]]*$ ]] && [[ -n "$current_section" ]]; then
        # Empty line - add to current section
        case "$current_section" in
            input) input_file="$input_file"$'\n' ;;
            expected_stdout) expected_stdout="$expected_stdout"$'\n' ;;
            expected_stderr) expected_stderr="$expected_stderr"$'\n' ;;
        esac
    elif [[ -n "$current_section" ]]; then
        # Content line - remove leading indentation
        if [[ $line =~ ^[[:space:]]+(.*)$ ]]; then
            content="${BASH_REMATCH[1]}"
        else
            content="$line"
        fi
        
        case "$current_section" in
            input) 
                if [ -n "$input_file" ]; then
                    input_file="$input_file"$'\n'"$content"
                else
                    input_file="$content"
                fi
                ;;
            expected_stdout)
                if [ -n "$expected_stdout" ]; then
                    expected_stdout="$expected_stdout"$'\n'"$content"
                else
                    expected_stdout="$content"
                fi
                ;;
            expected_stderr)
                if [ -n "$expected_stderr" ]; then
                    expected_stderr="$expected_stderr"$'\n'"$content"
                else
                    expected_stderr="$content"
                fi
                ;;
        esac
    fi
done < "$TEST_FILE"

# Run the last test
run_test

echo "All tests completed"