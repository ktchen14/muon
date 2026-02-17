#!/bin/bash

CLANG_FORMAT="docker run --rm -v $(cd "$(dirname "$0")/.." && pwd):/work muon-clang-format"

for f in \
  header/muon.h \
  header/**/*.h \
  source/engine/*.c \
  ; do
  diff --color -u "$f" <($CLANG_FORMAT --style=file "$f")
done
