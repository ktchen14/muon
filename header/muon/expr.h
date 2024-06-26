#ifndef MU_EXPR_H
#define MU_EXPR_H

#include "expr/common.h"   // IWYU pragma: export
#include "expr/access.h"   // IWYU pragma: export
#include "expr/integer.h"  // IWYU pragma: export
#include "expr/member.h"   // IWYU pragma: export
#include "expr/record.h"   // IWYU pragma: export
#include "expr/vector.h"   // IWYU pragma: export
#include "expr/zero.h"     // IWYU pragma: export

void mu_expr_debug(const mu_expr_t *expr) __attribute__((nonnull));

#endif /* MU_EXPR_H */
