#!/bin/bash

MUON_FORMAT="$(dirname "$0")/muon-format"

for f in \
  header/muon.h \
  header/**/*.h \
  source/engine/*.c \
  source/engine/*.h \
  ; do
  diff --color -u "$f" <("$MUON_FORMAT" "$f")
done
