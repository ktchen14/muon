#include "coercion.h"

#include <stddef.h>

#include "../stator/debug.h"

void mu_coercion_debug(const mu_coercion_t *coercion) {
  switch (coercion->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_COERCION: \
      mu_##lower##_coercion_debug((const mu_##lower##_coercion_t *) coercion); \
      return;
    MU_EACH_COERCION_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

void mu_id_coercion_debug(const mu_id_coercion_t *coercion) {
  fprintf(stderr, PRIsKIND "()", DEBUG_KIND("IdCoercion"));
}

void mu_simple_coercion_debug(const mu_simple_coercion_t *coercion) {
  fprintf(stderr, PRIsKIND "()", DEBUG_KIND("SimpleCoercion"));
}

void mu_join_coercion_debug(const mu_join_coercion_t *coercion) {
  fprintf(stderr, PRIsKIND "(i = %zu)",
      DEBUG_KIND("JoinCoercion"), coercion->i);
}

void mu_unjoin_coercion_debug(const mu_unjoin_coercion_t *coercion) {
  fprintf(stderr, PRIsKIND "(", DEBUG_KIND("UnjoinCoercion"));
  for (size_t i = 0; i < coercion->argc; i++) {
    if (i > 0)
      fprintf(stderr, ", ");
    mu_coercion_debug(coercion->argv[i]);
  }
  fprintf(stderr, ")");
}
