#ifndef MU_STMT_CONSTANT_I
#define MU_STMT_CONSTANT_I

#include <muon/stmt/constant.h>  // IWYU pragma: export

#include "common.h"

#include <stddef.h>

__attribute__((const, nonnull))
static inline size_t constant_stmt_size(const mu_constant_stmt_t *stmt) {
  return sizeof(mu_constant_stmt_t);
}

#endif /* MU_STMT_CONSTANT_I */
