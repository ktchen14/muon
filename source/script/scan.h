#ifndef MUON_SCRIPT_SCAN_H_INCLUDED
#define MUON_SCRIPT_SCAN_H_INCLUDED

#include "syntax.h"

typedef unsigned char YYCTYPE;

typedef struct {
  size_t cursor;  ///< location of the active character
  size_t marker;
  int condition;  ///< Re2c condition type
} scan_t;

/// Scan and return the next symbol in the @a buffer
yytoken_kind_t scan_next(
    const YYCTYPE *restrict buffer, scan_t *scan, YYSTYPE *yylval, YYLTYPE *yylloc)
  __attribute__((nonnull));

#endif