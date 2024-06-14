#ifndef MU_READER_SCAN_I
#define MU_READER_SCAN_I

#include <stddef.h>

typedef struct {
  size_t offset, line, column;
} cursor_t;

typedef struct {
  cursor_t cursor;  ///< location of the active character
  cursor_t symbol;  ///< location of the active symbol
  cursor_t marker;
  enum YYCONDTYPE condition;
} scan_t;

#endif /* MU_READER_SCAN_I */
