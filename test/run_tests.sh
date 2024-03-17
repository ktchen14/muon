#!/bin/bash -e

[ $# -eq 2 ] || { echo "Usage: $0 MUON TEST" 1>&2; exit 1; }

MUON="$1"
exec < "$2"

run_test() {
  echo "Test: $(basename "$root")"

  # Strip whitespace from all files
  for i in source; do
    sed -i '' -e '/./,$!d' -e :a -e '/^\s*$/N;/\n\s*$/ba' -e 's/\n\s*$//' "$root/$i" 2>/dev/null || true
  done

  set +e
  "$MUON" "$root/source" > "$root/stdout" 2> "$root/stderr"
  set -e

  # Strip whitespace from all files
  for i in stdout stderr stdout.expect stderr.expect; do
    sed -i '' -e '/./,$!d' -e :a -e '/^\s*$/N;/\n\s*$/ba' -e 's/\n\s*$//' "$root/$i" 2>/dev/null || true
  done

  [ -f "$root/stdout.expect" ] && diff -u "$root"/stdout{,.expect}
  [ -f "$root/stderr.expect" ] && diff -u "$root"/stderr{,.expect}
}

while IFS= read -r line; do
  if [[ $line =~ ^Test:\ (.+)$ ]]; then
    [ -n "$root" ] && run_test

    root="result/${BASH_REMATCH[1]}"
    rm -rf "$root" && mkdir -p "$root"

    section=source

  elif [[ $line =~ ^([[:space:]]*)Expected\ Stdout:$ ]]; then
    section=stdout.expect

  elif [[ $line =~ ^([[:space:]]*)Expected\ Stderr:$ ]]; then
    section=stderr.expect

  elif [[ $line =~ ^[[:space:]]*$ ]] && [[ -n "$section" ]]; then
    printf '%s\n' "$line" >> "$root/$section"

  elif [[ -n "$section" ]]; then
    printf '%s\n' "${line#  }" >> "$root/$section"
  fi
done

[ -n "$root" ] && run_test
