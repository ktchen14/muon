#include "stmt.h"

#include <assert.h>

void mu_stmt_debug(const mu_stmt_t *stmt) {
  switch (stmt->kind) {
    case MU_CONSTANT_STMT: 
      mu_constant_stmt_debug((const mu_constant_stmt_t *) stmt);
      break;
    case MU_TYPE_STMT:
      mu_type_stmt_debug((const mu_type_stmt_t *) stmt);
      break;
  }
}
