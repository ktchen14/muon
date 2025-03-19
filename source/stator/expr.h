#ifndef MU_STATOR_EXPR_I
#define MU_STATOR_EXPR_I

#include <muon/stator/expr.h>  // IWYU pragma: export

#include <stddef.h>

mu_record_expr_t *record_expr_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_record_expr_t *record_expr_activate(mu_record_expr_t *expr)
  __attribute__((nonnull));

mu_switch_expr_t *switch_expr_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_switch_expr_t *switch_expr_activate(mu_switch_expr_t *expr)
  __attribute__((nonnull));

mu_sequence_expr_t *sequence_expr_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_sequence_expr_t *sequence_expr_activate(mu_sequence_expr_t *expr)
  __attribute__((nonnull));

#endif /* MU_STATOR_EXPR_I */
