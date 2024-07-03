#ifndef MU_NODE_H
#define MU_NODE_H

#include "node/common.h"         // IWYU pragma: export

#include "node/access_expr.h"    // IWYU pragma: export
#include "node/integer_expr.h"   // IWYU pragma: export
#include "node/member_expr.h"    // IWYU pragma: export
#include "node/record_expr.h"    // IWYU pragma: export
#include "node/vector_expr.h"    // IWYU pragma: export
#include "node/zero_expr.h"      // IWYU pragma: export

#include "node/integer_sign.h"   // IWYU pragma: export
#include "node/member_sign.h"    // IWYU pragma: export
#include "node/name_sign.h"      // IWYU pragma: export
#include "node/record_sign.h"    // IWYU pragma: export
#include "node/variable_sign.h"  // IWYU pragma: export
#include "node/vector_sign.h"    // IWYU pragma: export

#include "node/constant_stmt.h"  // IWYU pragma: export
#include "node/type_stmt.h"      // IWYU pragma: export

/// Emit debugging information on the abstract @a node to the debug stream
void mu_node_debug(const mu_node_t *node) __attribute__((nonnull));

/// Emit debugging information on the abstract @a expr to the debug stream
void mu_expr_debug(const mu_expr_t *expr) __attribute__((nonnull));

/// Emit debugging information on the abstract @a sign to the debug stream
void mu_sign_debug(const mu_sign_t *sign) __attribute__((nonnull));

/// Emit debugging information on the abstract @a stmt to the debug stream
void mu_stmt_debug(const mu_stmt_t *stmt) __attribute__((nonnull));

#endif /* MU_NODE_H */
