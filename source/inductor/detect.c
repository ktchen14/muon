#include "detect.h"

#include "../common.h"
#include "../engine.h"
#include "../status.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
  MuonName *name;
  MuonNode *node;
} item_t;

typedef struct roster_t roster_t;
struct roster_t {
  roster_t *parent;
  MuonNode *origin;
  size_t length;
  size_t volume;
  item_t data[];
};

detect_t *detect_initialize(
    detect_t *detect, const MuonEngine *engine, mu_status_t *status) {
  size_t length = as_engine(engine)->node_number;

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
    roster_t *roster, size_t volume, MuonNode *origin) {
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
static MuonNode *roster_search(roster_t *roster, MuonName *name) {
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
    roster_t *roster, MuonName *name, MuonNode *node) {
  assert(roster->length < roster->volume);
  item_t item = { .name = name, .node = node };
  roster->data[roster->length++] = item;
}

__attribute__((nonnull))
static void view_announce(roster_t *roster, MuonView *root) {
  MuonNode *node = &root->as_node, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    switch ON_ABSTRACT_OBJECT(node) {
      case IS_CONCRETE_NODE(MuonVariableView *variable_view)
        announce(roster, variable_view->name, &variable_view->as_node);
        break;

      default: break;
    }
  } while ((node = node_return(node)) != NULL);
}

__attribute__((nonnull(2)))
roster_t *handle_script(roster_t *roster, MuonScript *script) {
  size_t announce_length = 0;
  for (size_t i = 0; i < script->argc; i++) {
    MuonStmt *stmt = script->argv[i];
    announce_length += node_announce_length(&stmt->as_node);
  }

  if ((roster = roster_create(roster, announce_length, &script->as_node)) == NULL)
    return NULL;

  for (size_t i = 0; i < script->argc; i++) {
    MuonStmt *stmt = script->argv[i];

    switch ON_ABSTRACT_OBJECT(stmt) {
      case IS_CONCRETE_NODE(MuonDatatypeStmt *datatype_stmt)
        announce(roster, datatype_stmt->name, &datatype_stmt->as_node);
        for (size_t j = 0; j < datatype_stmt->argc; j++) {
          MuonDatatypeOption *option = datatype_stmt->argv[j];
          announce(roster, option->name, &option->as_node);
        }
        break;

      case IS_CONCRETE_NODE(MuonDefineStmt *define_stmt)
        announce(roster, define_stmt->name, &define_stmt->as_node);
        break;

      default: break;
    }
  }

  return roster;
}

__attribute__((nonnull(2)))
roster_t *handle_sequence_expr(
    roster_t *roster, MuonSequenceExpr *sequence_expr) {
  size_t announce_length = 0;
  for (size_t i = 0; i < sequence_expr->argc; i++) {
    MuonStmt *stmt = sequence_expr->argv[i];
    announce_length += node_announce_length(&stmt->as_node);
  }

  if ((roster = roster_create(roster, announce_length, &sequence_expr->as_node)) == NULL)
    return NULL;

  for (size_t i = 0; i < sequence_expr->argc; i++) {
    MuonStmt *stmt = sequence_expr->argv[i];

    switch ON_ABSTRACT_OBJECT(stmt) {
      case IS_CONCRETE_NODE(MuonDatatypeStmt *datatype_stmt)
        announce(roster, datatype_stmt->name, &datatype_stmt->as_node);
        for (size_t j = 0; j < datatype_stmt->argc; j++) {
          MuonDatatypeOption *option = datatype_stmt->argv[j];
          announce(roster, option->name, &option->as_node);
        }
        break;

      case IS_CONCRETE_NODE(MuonDefineStmt *define_stmt)
        announce(roster, define_stmt->name, &define_stmt->as_node);
        break;

      default: break;
    }
  }

  return roster;
}

detect_t *detect_node(detect_t *detect, MuonNode *root) {
  MuonScript *script = muon_node_cast(root, script);
  assert(script != NULL);

  roster_t *roster = NULL;
  if ((roster = handle_script(roster, script)) == NULL)
    return NULL;

  MuonNode *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL) {
      switch ON_ABSTRACT_OBJECT(node) {
        case IS_CONCRETE_NODE(MuonLambdaExpr *lambda_expr) {
          size_t length = node_announce_length(&lambda_expr->argument->as_node);

          roster_t *next_roster;
          if ((next_roster = roster_create(roster, length, &lambda_expr->as_node)) == NULL)
            return NULL;
          roster = next_roster;
          view_announce(roster, lambda_expr->argument);
          break;
        }

        case IS_CONCRETE_NODE(MuonSequenceExpr *sequence_expr) {
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
      case IS_CONCRETE_NODE(MuonNameExpr *name_expr) {
        MuonNode *target = roster_search(roster, name_expr->name);
        assert(target != NULL);
        detect->result->data[name_expr->as_node.id] = target;
        break;
      }

      case IS_CONCRETE_NODE(MuonSwitchCase *switch_case) {
        MuonNode *target = roster_search(roster, switch_case->name);
        assert(target != NULL);
        detect->result->data[switch_case->as_node.id] = target;
        break;
      }

      case IS_CONCRETE_NODE(MuonNameSign *name_sign) {
        MuonNode *target = roster_search(roster, name_sign->name);
        assert(target != NULL);
        detect->result->data[name_sign->as_node.id] = target;
        break;
      }

      default: break;
    }
  } while ((node = node_return(node)) != NULL);

  return detect;
}
