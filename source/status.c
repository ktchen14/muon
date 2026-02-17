#include "status.h"

#include <stdarg.h>
#include <stdio.h>

const mu_memo_t memo = {0};

const mu_memo_t *mu_memo(
    mu_status_t *status, const char *restrict format, ...) {
  va_list variadic;
  va_start(variadic, format);
  vfprintf(stderr, format, variadic);
  va_end(variadic);

  return &memo;
}
