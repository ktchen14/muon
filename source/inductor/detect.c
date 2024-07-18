#include "detect.h"

#include "../common.h"
#include "../script.h"
#include "../stator.h"
#include "../status.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

detect_t *detect_initialize(
    detect_t *detect, const mu_engine_t *engine, mu_status_t *status) {
  size_t length = engine->node_number;

  size_t size;
  if (rare((size = struct_size(detect_result_t, data, length)) == 0))
    return errno = ENOMEM, NULL;

  detect_result_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (detect_result_t) { .engine = engine, .length = length };
  for (size_t i = 0; i < length; result->data[i++] = NULL);

  *detect = (detect_t) { .status = status, .result = result };
  return detect;
}

const mu_stmt_t *script_get(const mu_script_t *script, const mu_name_t *name) {
  for (size_t i = 0; i < script->argc; i++) {
    const mu_stmt_t *stmt = script->argv[i];

    const mu_define_stmt_t *define_stmt;
    if ((define_stmt = mu_stmt_cast(stmt, define_stmt)) == NULL)
      continue;

    if (define_stmt->name != name)
      continue;

    return &define_stmt->as_stmt;
  }

  return NULL;
}

detect_t *detect_script(const mu_script_t *script, detect_t *detect) {
  for (size_t i = 0; i < script->argc; i++) {
    const mu_node_t *node = &script->argv[i]->as_node;

    do {
      const mu_node_t *next;
      while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
        node = node_continue(node, next);

      const mu_name_expr_t *name_expr;
      if ((name_expr = mu_node_cast(node, name_expr)) == NULL)
        continue;

      const mu_node_t *anterior = node_cursor(node)->anterior;
      for (; anterior != NULL; anterior = node_cursor(anterior)->anterior) {
        const mu_lambda_expr_t *lambda_expr;
        if ((lambda_expr = mu_node_cast(anterior, lambda_expr)) == NULL)
          continue;

        const mu_variable_view_t *view = lambda_expr->argument;
        if (view->name != name_expr->name)
          continue;

        detect->result->data[name_expr->as_stator.id] = &view->as_node;
        goto next;
      }

      const mu_stmt_t *target;
      target = script_get(script, name_expr->name);
      assert(target != NULL);
      detect->result->data[name_expr->as_stator.id] = &target->as_node;

    next:;
    } while ((node = node_return(node)) != NULL);
  }

  return detect;
}
