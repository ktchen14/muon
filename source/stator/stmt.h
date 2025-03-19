#ifndef MU_STATOR_STMT_I
#define MU_STATOR_STMT_I

#include <muon/stator/stmt.h>  // IWYU pragma: export

#include <stddef.h>

mu_datatype_stmt_t *datatype_stmt_allocate(mu_engine_t *engine, size_t argc)
  __attribute__((malloc, nonnull));

const mu_datatype_stmt_t *datatype_stmt_activate(
    mu_datatype_stmt_t *stmt, const mu_name_t *name)
  __attribute__((nonnull));

#endif /* MU_STATOR_STMT_I */
