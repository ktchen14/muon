#ifndef MU_STATOR_H
#define MU_STATOR_H

#include "stator/common.h"         // IWYU pragma: export
#include "stator/name.h"           // IWYU pragma: export
#include "stator/node.h"           // IWYU pragma: export
#include "stator/type.h"           // IWYU pragma: export

#include "stator/access_expr.h"    // IWYU pragma: export
#include "stator/integer_expr.h"   // IWYU pragma: export
#include "stator/member_expr.h"    // IWYU pragma: export
#include "stator/name_expr.h"      // IWYU pragma: export
#include "stator/record_expr.h"    // IWYU pragma: export
#include "stator/vector_expr.h"    // IWYU pragma: export
#include "stator/zero_expr.h"      // IWYU pragma: export

#include "stator/integer_sign.h"   // IWYU pragma: export
#include "stator/member_sign.h"    // IWYU pragma: export
#include "stator/name_sign.h"      // IWYU pragma: export
#include "stator/record_sign.h"    // IWYU pragma: export
#include "stator/variable_sign.h"  // IWYU pragma: export
#include "stator/vector_sign.h"    // IWYU pragma: export

#include "stator/constant_stmt.h"  // IWYU pragma: export
#include "stator/type_stmt.h"      // IWYU pragma: export

#include "stator/integer_type.h"   // IWYU pragma: export
#include "stator/vector_type.h"    // IWYU pragma: export

/// Emit debugging information on the abstract @a node to the debug stream
void mu_node_debug(const mu_node_t *node) __attribute__((nonnull));

/// Emit debugging information on the abstract @a expr to the debug stream
void mu_expr_debug(const mu_expr_t *expr) __attribute__((nonnull));

/// Emit debugging information on the abstract @a sign to the debug stream
void mu_sign_debug(const mu_sign_t *sign) __attribute__((nonnull));

/// Emit debugging information on the abstract @a stmt to the debug stream
void mu_stmt_debug(const mu_stmt_t *stmt) __attribute__((nonnull));

/// Emit debugging information on the abstract @a type to the debug stream
void mu_type_debug(const mu_type_t *type) __attribute__((nonnull));

#endif /* MU_STATOR_H */
