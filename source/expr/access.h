#ifndef MU_EXPR_ACCESS_I
#define MU_EXPR_ACCESS_I

#include <muon/expr/access.h>  // IWYU pragma: export

#include "common.h"

__attribute__((const, nonnull))
static inline size_t access_expr_size(const mu_access_expr_t *expr) {
  return sizeof(mu_access_expr_t);
}

#endif /* MU_EXPR_ACCESS_I */
