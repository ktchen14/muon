#include "detect.h"

#include "../common.h"
#include "../engine.h"
#include "../status.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
  mu_name_t *name;
  mu_node_t *node;
} item_t;

typedef struct roster_t roster_t;
struct roster_t {
  roster_t *parent;
  mu_node_t *origin;
  size_t length;
  size_t volume;
  item_t data[];
};

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

__attribute__((malloc))
static roster_t *roster_create(
    roster_t *roster, size_t volume, mu_node_t *origin) {
  size_t size;
  if (rare((size = struct_size(roster_t, data, volume)) == 0))
    return errno = ENOMEM, NULL;

  roster_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (roster_t) { .parent = roster, .volume = volume, .origin = origin };
  return result;
}

__attribute__((nonnull))
static mu_node_t *roster_search(roster_t *roster, mu_name_t *name) {
  do {
    for (size_t i = 0; i < roster->length; i++) {
      if (roster->data[i].name == name)
        return roster->data[i].node;
    }
  } while ((roster = roster->parent) != NULL);

  return NULL;
}

__attribute__((nonnull))
static inline void announce(
    roster_t *roster, mu_name_t *name, mu_node_t *node) {
  assert(roster->length < roster->volume);
  item_t item = { .name = name, .node = node };
  roster->data[roster->length++] = item;
}

__attribute__((nonnull))
static void view_announce(roster_t *roster, const mu_view_t *root) {
  mu_node_t *node = &root->as_node, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    switch ON_ABSTRACT_OBJECT(node) {
      case IS_KIND_OF(variable_view):
        announce(roster, variable_view->name, &variable_view->as_node);
        break;

      default: break;
    }
  } while ((node = node_return(node)) != NULL);
}

__attribute__((nonnull(2)))
roster_t *handle_sequence_expr(
    roster_t *roster, const mu_sequence_expr_t *sequence_expr) {
  size_t announce_length = 0;
  for (size_t i = 0; i < sequence_expr->argc; i++) {
    const mu_stmt_t *stmt = sequence_expr->argv[i];
    announce_length += node_announce_length(&stmt->as_node);
  }

  if ((roster = roster_create(roster, announce_length, &sequence_expr->as_node)) == NULL)
    return NULL;

  for (size_t i = 0; i < sequence_expr->argc; i++) {
    const mu_stmt_t *stmt = sequence_expr->argv[i];

    switch ON_ABSTRACT_OBJECT(stmt) {
      case IS_KIND_OF(datatype_stmt):
        announce(roster, datatype_stmt->name, &datatype_stmt->as_node);
        for (size_t j = 0; j < datatype_stmt->argc; j++) {
          const mu_datatype_option_t *option = datatype_stmt->argv[j];
          announce(roster, option->name, &option->as_node);
        }
        break;

      case IS_KIND_OF(define_stmt):
        announce(roster, define_stmt->name, &define_stmt->as_node);
        break;

      default: break;
    }
  }

  return roster;
}

detect_t *detect_node(detect_t *detect, mu_node_t *root) {
  const mu_sequence_expr_t *sequence_expr = mu_node_cast(root, sequence_expr);
  assert(sequence_expr != NULL);

  roster_t *roster = NULL;
  if ((roster = handle_sequence_expr(roster, sequence_expr)) == NULL)
    return NULL;

  mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL) {
      switch ON_ABSTRACT_OBJECT(node) {
        case IS_KIND_OF(lambda_expr): {
          size_t length = node_announce_length(&lambda_expr->argument->as_node);

          roster_t *next_roster;
          if ((next_roster = roster_create(roster, length, &lambda_expr->as_node)) == NULL)
            return NULL;
          roster = next_roster;
          view_announce(roster, lambda_expr->argument);
          break;
        }

        case IS_KIND_OF(sequence_expr): {
          roster_t *next_roster;
          if ((next_roster = handle_sequence_expr(roster, sequence_expr)) == NULL)
            return NULL;
          roster = next_roster;
          break;
        }

        default: break;
      }

      node = node_continue(node, next);
    }

    if (node == roster->origin) {
      roster_t *next_roster = roster->parent;
      free(roster);
      roster = next_roster;
    }

    switch ON_ABSTRACT_OBJECT(node) {
      case IS_KIND_OF(name_expr): {
        mu_node_t *target = roster_search(roster, name_expr->name);
        assert(target != NULL);
        detect->result->data[name_expr->as_node.id] = target;
        break;
      }

      case IS_KIND_OF(switch_case): {
        mu_node_t *target = roster_search(roster, switch_case->name);
        assert(target != NULL);
        detect->result->data[switch_case->as_node.id] = target;
        break;
      }

      case IS_KIND_OF(name_sign): {
        mu_node_t *target = roster_search(roster, name_sign->name);
        assert(target != NULL);
        detect->result->data[name_sign->as_node.id] = target;
        break;
      }

      default: break;
    }
  } while ((node = node_return(node)) != NULL);

  return detect;
}
