#include "stmt.h"

#include <assert.h>

void mu_stmt_debug(const mu_stmt_t *stmt) {
  switch (stmt->kind) {
    case MU_CONSTANT_STMT: 
      return mu_constant_stmt_debug((const mu_constant_stmt_t *) stmt);
  }
  assert(0);
}
