#ifndef MU_STATOR_H
#define MU_STATOR_H

#include "stator/common.h"  // IWYU pragma: export
#include "stator/engine.h"  // IWYU pragma: export
#include "stator/name.h"    // IWYU pragma: export
#include "stator/node.h"    // IWYU pragma: export

#include <stddef.h>

#define mu_stator_cast(abstract, concrete) ({ \
    const mu_stator_t *_abstract = (abstract); \
    typeof(concrete) _concrete; \
    \
    mu_stator_kind_t _kind = _abstract->kind; \
    _Bool _castable = _Generic(_concrete, \
      const mu_name_t *: _kind == MU_NAME_STATOR, \
      const mu_node_t *: 0 MU_EACH_NODE_KIND(MU_CAST_EMIT, _STATOR), \
      const mu_expr_t *: 0 MU_EACH_EXPR_KIND(MU_CAST_EMIT, _EXPR_STATOR), \
      const mu_sign_t *: 0 MU_EACH_SIGN_KIND(MU_CAST_EMIT, _SIGN_STATOR), \
      const mu_stmt_t *: 0 MU_EACH_STMT_KIND(MU_CAST_EMIT, _STMT_STATOR), \
      const mu_view_t *: 0 MU_EACH_VIEW_KIND(MU_CAST_EMIT, _VIEW_STATOR), \
      const mu_type_t *: 0 MU_EACH_TYPE_KIND(MU_CAST_EMIT, _TYPE_STATOR) \
      MU_EACH_EXPR_KIND(MU_EXPR_CAST_EMIT, _STATOR) \
      MU_EACH_SIGN_KIND(MU_SIGN_CAST_EMIT, _STATOR) \
      MU_EACH_STMT_KIND(MU_STMT_CAST_EMIT, _STATOR) \
      MU_EACH_VIEW_KIND(MU_VIEW_CAST_EMIT, _STATOR) \
      MU_EACH_TYPE_KIND(MU_TYPE_CAST_EMIT, _STATOR)); \
    _castable ? (typeof(_concrete)) _abstract : NULL; \
  })

#endif /* MU_STATOR_H */
