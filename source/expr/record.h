#ifndef MU_EXPR_RECORD_I
#define MU_EXPR_RECORD_I

#include <muon/expr/record.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((nonnull, pure))
static inline size_t record_expr_size(const mu_record_expr_t *expr) {
  return extant_size(mu_record_expr_t, argv, expr->argc);
}

void record_expr_debug(const mu_record_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_EXPR_RECORD_I */
