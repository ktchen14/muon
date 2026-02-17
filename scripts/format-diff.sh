#!/bin/bash
for f in \
  header/muon.h \
  header/muon/common.h \
  header/muon/engine.h \
  header/muon/engine/common.h \
  header/muon/engine/name.h \
  header/muon/engine/node.h \
  header/muon/inductor.h \
  header/muon/inductor/core.h \
  header/muon/inductor/coercion.h \
  header/muon/inductor/type.h \
  header/muon/scan.h \
  header/muon/status.h \
  source/engine/node.c \
  ; do
  diff --color -u "$f" <(clang-format --style=file "$f")
done
