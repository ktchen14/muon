#include "expr.h"

#include <assert.h>

void mu_expr_debug(const mu_expr_t *expr) {
  switch (expr->kind) { MU_EACH_EXPR_KIND(MU_ABSTRACT_EXPR_CALL, mu, debug) }
}
