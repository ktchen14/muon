#include "detect.h"

#include "../common.h"
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

const mu_node_t *sequence_expr_get(
    const mu_sequence_expr_t *expr, const mu_name_t *name) {
  for (size_t i = 0; i < expr->argc; i++) {
    const mu_stmt_t *stmt = expr->argv[i];

    const mu_define_stmt_t *define_stmt;
    if ((define_stmt = mu_stmt_cast(stmt, define_stmt)) == NULL)
      continue;

    if (define_stmt->name != name)
      continue;

    return &define_stmt->as_node;
  }

  for (size_t j = 0; j < expr->argc; j++) {
    const mu_stmt_t *stmt = expr->argv[j];

    const mu_datatype_stmt_t *datatype_stmt;
    if ((datatype_stmt = mu_stmt_cast(stmt, datatype_stmt)) != NULL) {
      for (size_t i = 0; i < datatype_stmt->argc; i++) {
        const mu_datatype_option_t *option = datatype_stmt->argv[i];
        if (option->name == name)
          return &option->as_node;
      }
    }
  }

  return NULL;
}

detect_t *detect_node(detect_t *detect, const mu_node_t *node) {
  do {
    const mu_node_t *next;
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    const mu_node_t *source;
    const mu_name_t *name;

    const mu_name_expr_t *name_expr;
    const mu_switch_case_t *switch_case;
    if ((name_expr = mu_node_cast(node, name_expr)) != NULL) {
      name = name_expr->name;
      source = &name_expr->as_node;
    } else if ((switch_case = mu_node_cast(node, switch_case)) != NULL) {
      name = switch_case->name;
      source = &switch_case->as_node;
    } else
      continue;

    const mu_node_t *anterior = node_cursor(node)->anterior;
    for (; anterior != NULL; anterior = node_cursor(anterior)->anterior) {
      const mu_sequence_expr_t *sequence_expr;
      if ((sequence_expr = mu_node_cast(anterior, sequence_expr)) != NULL) {
        const mu_node_t *target;
        if ((target = sequence_expr_get(sequence_expr, name)) != NULL) {
          detect->result->data[source->id] = target;
          goto next;
        }

        continue;
      }

      const mu_lambda_expr_t *lambda_expr;
      if ((lambda_expr = mu_node_cast(anterior, lambda_expr)) != NULL) {
        const mu_view_t *view = lambda_expr->argument;
        const mu_variable_view_t *variable_view = mu_view_cast(view, variable_view);
        assert(variable_view != NULL);
        if (variable_view->name == name) {
          detect->result->data[source->id] = &variable_view->as_node;
          goto next;
        }
        continue;
      }
    }

    assert(0);

  next:;
  } while ((node = node_return(node)) != NULL);

  return detect;
}
