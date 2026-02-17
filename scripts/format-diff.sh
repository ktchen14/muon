#!/usr/bin/env bash

shopt -s globstar

MUON_FORMAT="$(dirname "$0")/muon-format"
MUON_FORMAT=clang-format

for f in header/**/*.h source/**/*.[ch]; do
# for f in header/**/*.h; do
  diff --color -u "$f" <("$MUON_FORMAT" "$f")
done
