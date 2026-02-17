#!/bin/bash

MUON_FORMAT="$(dirname "$0")/muon-format"

for f in \
  header/muon.h \
  header/**/*.h \
  source/engine/*.[ch] \
  source/author/*.[ch] \
  source/inductor/*.[ch] \
  ; do
  diff --color -u "$f" <("$MUON_FORMAT" "$f")
done
