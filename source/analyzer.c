#include "engine.h"
#include "expr.h"
#include "name.h"
#include "node.h"
#include "script.h"
#include "stmt.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

const mu_stmt_t *script_get(const mu_script_t *script, const mu_name_t *name) {
  for (size_t i = 0; i < script->argc; i++) {
    const mu_stmt_t *stmt = script->argv[i];
    if (stmt->kind != MU_CONSTANT_STMT)
      continue;

    const mu_constant_stmt_t *constant_stmt = (const mu_constant_stmt_t *) stmt;
    if (constant_stmt->name != name)
      continue;

    return &constant_stmt->as_stmt;
  }

  return NULL;
}

const mu_stmt_t *const *resolve_names(mu_engine_t *engine, const mu_script_t *script) {
  size_t size = sizeof(const mu_stmt_t *[engine->stator_id]);
  const mu_stmt_t **result = malloc(size);
  for (size_t i = 0; i < engine->stator_id; i++)
    result[i] = 0;

  for (size_t i = 0; i < script->argc; i++) {
    const mu_node_t *node = &script->argv[i]->as_node;

    do {
      const mu_node_t *next;
      while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
        node = node_continue(node, next);

      mu_node_kind_t kind = node->kind;
      if (kind == MU_NAME_EXPR_NODE) {
        const mu_name_expr_t *expr = (const mu_name_expr_t *) node;

        const mu_stmt_t *target;
        target = script_get(script, expr->name);
        assert(target != NULL);

        result[expr->as_stator.id] = target;
      }
    } while ((node = node_return(node)) != NULL);
  }

  return result;
}
