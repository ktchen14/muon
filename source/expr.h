#ifndef MU_EXPR_I
#define MU_EXPR_I

#include <muon/expr.h>     // IWYU pragma: export

#include "expr/access.h"   // IWYU pragma: export
#include "expr/integer.h"  // IWYU pragma: export
#include "expr/member.h"   // IWYU pragma: export
#include "expr/name.h"     // IWYU pragma: export
#include "expr/record.h"   // IWYU pragma: export
#include "expr/vector.h"   // IWYU pragma: export
#include "expr/zero.h"     // IWYU pragma: export

/**
 * @brief Emit a @c case in an abstract call on an expr function
 */
#define MU_ABSTRACT_EXPR_CALL(lower, upper, t, variable, prefix, name, ...) \
  case MU_##upper##_EXPR: \
    prefix##_##lower##_expr_##name((const mu_##lower##_expr_t *) variable, ##__VA_ARGS__); \
    break;

#endif /* MU_EXPR_I */
