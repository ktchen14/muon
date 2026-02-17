#!/bin/bash

MUON_FORMAT="$(dirname "$0")/muon-format"

for f in \
  header/muon.h \
  header/**/*.h \
  source/engine/*.[ch] \
  source/author/*.[ch] \
  ; do
  diff --color -u "$f" <("$MUON_FORMAT" -i "$f")
done
