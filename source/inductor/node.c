#include "../common.h"
#include "../engine.h"
#include "coercion.h"
#include "core.h"
#include "detect.h"
#include "induce.h"
#include "type.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static const mu_type_t *node_induce(induce_t *induce, MuonNode *node);

const mu_type_t *induce_node(induce_t *induce, MuonNode *root) {
  assert(induce->scheme == NULL);
  induce->scheme = &(mu_scheme_t) { .induce = induce, .id = induce->type_number };

  assert(root->id < induce->node_length);

  MuonNode *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL) {
      node = node_continue(node, next);

      MuonDatatypeStmt *datatype_stmt;
      if ((datatype_stmt = mu_node_cast(node, datatype_stmt)) != NULL) {
        const mu_core_t *core;
        if ((core = mu_simple_core(induce, datatype_stmt->name)) == NULL)
          return NULL;
        induce->core[induce->core_length++] = core;
        induce->datatype_core = core;
      }

      if (node->kind != MU_DEFINE_STMT_NODE)
        continue;

      mu_scheme_t *scheme;
      if ((scheme = mu_scheme(induce->scheme)) == NULL)
        return NULL;
      induce->scheme = scheme;
    }

    // Induce the type of the node
    const mu_type_t *type;
    if ((type = node_induce(induce, node)) == NULL)
      return NULL;

    if (node->kind == MU_DEFINE_STMT_NODE) {
      mu_scheme_t *parent = induce->scheme->parent;
      free(induce->scheme);
      induce->scheme = parent;
    }

    induce->result[node->id].source_type = type;
  } while ((node = node_return(node)) != NULL);

  return evince_type(induce, root);
}

__attribute__((nonnull)) static const mu_type_t *access_expr_induce(
    induce_t *induce, MuonAccessExpr *expr) {
  const mu_variable_type_t *variable_type;
  if ((variable_type = mu_variable_type(induce)) == NULL)
    return NULL;

  const mu_core_t *core;
  if ((core = single_record_core(induce, expr->name)) == NULL)
    return NULL;
  assert(core->kind == MU_RECORD_CORE);

  const mu_type_t *record_argv[] = { &variable_type->as_type };
  const mu_core_type_t *record_type;
  if ((record_type = mu_core_type(induce, core, record_argv)) == NULL)
    return NULL;

  const mu_type_t *argv[] = { &record_type->as_type, &variable_type->as_type };
  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argv[0], argv[1])) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *boolean_expr_induce(
    induce_t *induce, MuonBooleanExpr *expr) {
  const mu_core_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *cast_expr_induce(
    induce_t *induce, MuonCastExpr *expr) {
  const mu_type_t *sign_type = evince_type(induce, &expr->sign->as_node);
  const mu_type_t *matter_type = evince_type(induce, &expr->matter->as_node);

  const mu_coercion_t *coercion;
  if ((coercion = ensure_coercion(induce, matter_type, sign_type)) == NULL)
    return NULL;
  assign_coercion_to_node(induce, &expr->matter->as_node, coercion, sign_type);

  return sign_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_expr_induce(
    induce_t *induce, MuonIntegerExpr *expr) {
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *invoke_expr_induce(
    induce_t *induce, MuonInvokeExpr *expr) {
  const mu_type_t *operator_type, *argument_type;
  operator_type = evince_type(induce, &expr->operator->as_node);
  argument_type = evince_type(induce, &expr->argument->as_node);

  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;

  const mu_type_t *argv[] = { argument_type, &result->as_type };
  const mu_core_type_t *lambda_type;
  if ((lambda_type = mu_lambda_type(induce, argv[0], argv[1])) == NULL)
    return NULL;

  const mu_type_t *target = &lambda_type->as_type;
  const mu_coercion_t *coercion;
  if ((coercion = ensure_coercion(induce, operator_type, target)) == NULL)
    return NULL;
  assign_coercion_to_node(induce, &expr->operator->as_node, coercion, target);

  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *lambda_expr_induce(
    induce_t *induce, MuonLambdaExpr *expr) {
  const mu_type_t *argument_type = evince_type(induce, &expr->argument->as_node);
  const mu_type_t *output_type = evince_type(induce, &expr->matter->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *name_expr_induce(
    induce_t *induce, MuonNameExpr *expr) {
  MuonNode *target = detect_evince(induce->detect, &expr->as_node);
  assert(target != NULL);
  return evince_type(induce, target);
}

__attribute__((nonnull)) static const mu_type_t *native_expr_induce(
    induce_t *induce, MuonNativeExpr *expr) {
  const mu_core_type_t *integer_type;
  if ((integer_type = mu_integer_type(induce)) == NULL)
    return NULL;

  const mu_core_type_t *vector_type;
  if ((vector_type = mu_vector_type(induce, &integer_type->as_type)) == NULL)
    return NULL;

  const mu_type_t *argument_type = &vector_type->as_type;
  const mu_type_t *output_type = &integer_type->as_type;
  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *record_expr_induce(
    induce_t *induce, MuonRecordExpr *expr) {
  mu_core_t *core_allocation;
  if ((core_allocation = record_core_allocate(induce, expr->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    MuonName *name = expr->argv[i]->name;

    mu_core_member_t member = { .name = name };
    core_allocation->argv[i] = member;
  }
  /* qsort(&core_allocation->argv[j], expr->argc - j, sizeof(mu_expr_member_t), */
  /*     type_member_cmp); */
  // TODO: check for duplicates

  const mu_core_t *core;
  if (rare((core = record_core_activate(core_allocation)) == NULL))
    return NULL;

  mu_core_type_t *type_allocation;
  if ((type_allocation = core_type_allocate(induce, core)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++)
    type_allocation->argv[i] = evince_type(induce, &expr->argv[i]->as_node);

  const mu_core_type_t *result;
  if (rare((result = core_type_activate(type_allocation)) == NULL))
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *switch_case_induce(
    induce_t *induce, MuonSwitchCase *node) {
  MuonNode *target;
  if ((target = detect_evince(induce->detect, &node->as_node)) == NULL)
    abort();
  const mu_type_t *case_type = evince_type(induce, target);
  assert(case_type->kind != MU_SCHEME_TYPE);

  const mu_type_t *expr_type = evince_type(induce, &node->expr->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, case_type, expr_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *switch_expr_induce(
    induce_t *induce, MuonSwitchExpr *expr) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = evince_type(induce, &expr->argv[i]->as_node);

    const mu_coercion_t *coercion;
    if ((coercion = ensure_coercion(induce, type, &result->as_type)) == NULL)
      return NULL;
    assign_coercion_to_node(induce, &expr->argv[i]->as_node, coercion, &result->as_type);
  }

  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *sequence_expr_induce(
    induce_t *induce, MuonSequenceExpr *expr) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *vector_expr_induce(
    induce_t *induce, MuonVectorExpr *expr) {
  const mu_variable_type_t *matter_type;
  if ((matter_type = mu_variable_type(induce)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = evince_type(induce, &expr->argv[i]->as_node);

    const mu_coercion_t *coercion;
    if ((coercion = ensure_coercion(induce, type, &matter_type->as_type)) == NULL)
      return NULL;
    assign_coercion_to_node(induce, &expr->argv[i]->as_node, coercion, &matter_type->as_type);
  }

  const mu_core_type_t *result;
  if ((result = mu_vector_type(induce, &matter_type->as_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *boolean_sign_induce(
    induce_t *induce, MuonBooleanSign *sign) {
  const mu_core_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_sign_induce(
    induce_t *induce, MuonIntegerSign *sign) {
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *lambda_sign_induce(
    induce_t *induce, MuonLambdaSign *sign) {
  const mu_type_t *argument_type = evince_type(induce, &sign->argument->as_node);
  const mu_type_t *output_type = evince_type(induce, &sign->output->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *name_sign_induce(
    induce_t *induce, MuonNameSign *sign) {
  MuonNode *target = detect_evince(induce->detect, &sign->as_node);
  assert(target != NULL);
  return evince_type(induce, target);
}

__attribute__((nonnull)) static const mu_type_t *record_sign_induce(
    induce_t *induce, MuonRecordSign *sign) {
  assert(0);
}

__attribute__((nonnull)) static const mu_type_t *vector_sign_induce(
    induce_t *induce, MuonVectorSign *sign) {
  const mu_type_t *matter = evince_type(induce, &sign->matter->as_node);

  const mu_core_type_t *result;
  if ((result = mu_vector_type(induce, matter)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *coercion_stmt_induce(
    induce_t *induce, MuonCoercionStmt *stmt) {
  const mu_type_t *source_type = evince_type(induce, &stmt->source->as_node);
  const mu_type_t *target_type = evince_type(induce, &stmt->target->as_node);
  const mu_type_t *expr_type = evince_type(induce, &stmt->expr->as_node);

  const mu_core_type_t *lambda_type;
  if ((lambda_type = mu_lambda_type(induce, source_type, target_type)) == NULL)
    return NULL;

  // TODO: ensure that this isn't tautological
  const mu_coercion_t *coercion;
  if ((coercion = ensure_coercion(induce, expr_type, &lambda_type->as_type)) == NULL)
    return NULL;
  assign_coercion_to_node(induce, &stmt->expr->as_node, coercion, &lambda_type->as_type);

  const mu_core_type_t *source_core_type = mu_type_cast(source_type, source_core_type);
  assert(source_core_type != NULL);
  const mu_core_type_t *target_core_type = mu_type_cast(target_type, target_core_type);
  assert(target_core_type != NULL);

  const mu_instance_t *instance;
  if ((instance = mu_instance(source_core_type->core, target_core_type->core, stmt->expr)) == NULL)
    return NULL;
  induce->instance[induce->instance_length++] = instance;

  return &lambda_type->as_type;
}

__attribute__((nonnull)) static const mu_type_t *datatype_option_induce(
    induce_t *induce, MuonDatatypeOption *option) {
  const mu_core_t *core = induce->datatype_core;
  assert(core != NULL);

  const mu_core_type_t *result;
  if ((result = mu_core_type(induce, core, NULL)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *datatype_stmt_induce(
    induce_t *induce, MuonDatatypeStmt *stmt) {
  // TODO: No arguments supported for now
  const mu_core_t *core = induce->datatype_core;
  const mu_core_type_t *result;
  if ((result = mu_core_type(induce, core, NULL)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *define_stmt_induce(
    induce_t *induce, MuonDefineStmt *stmt) {
  const mu_type_t *expr_type = evince_type(induce, &stmt->expr->as_node);
  return generalize_type(induce, expr_type);
}

__attribute__((nonnull)) static const mu_type_t *record_view_induce(
    induce_t *induce, MuonRecordView *view) {
  mu_core_t *core_allocation;
  if ((core_allocation = record_core_allocate(induce, view->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < view->argc; i++) {
    MuonName *name = view->argv[i]->name;

    mu_core_member_t member = { .name = name };
    core_allocation->argv[i] = member;
  }
  /* qsort(&core_allocation->argv[j], view->argc - j, sizeof(mu_view_member_t), */
  /*     type_member_cmp); */
  // TODO: check for duplicates

  const mu_core_t *core;
  if (rare((core = record_core_activate(core_allocation)) == NULL))
    return NULL;

  mu_core_type_t *type_allocation;
  if ((type_allocation = core_type_allocate(induce, core)) == NULL)
    return NULL;

  for (size_t i = 0; i < view->argc; i++)
    type_allocation->argv[i] = evince_type(induce, &view->argv[i]->as_node);

  const mu_core_type_t *result;
  if (rare((result = core_type_activate(type_allocation)) == NULL))
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *variable_view_induce(
    induce_t *induce, MuonVariableView *view) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *expr_member_induce(
    induce_t *induce, MuonExprMember *member) {
  return evince_type(induce, &member->expr->as_node);
}

__attribute__((nonnull)) static const mu_type_t *view_member_induce(
    induce_t *induce, MuonViewMember *member) {
  return evince_type(induce, &member->view->as_node);
}

static const mu_type_t *node_induce(induce_t *induce, MuonNode *node) {
  switch ON_ABSTRACT_OBJECT(node) {
#define MU_EMIT(lower, upper, title) \
    case MU_##upper##_NODE: \
      return lower##_induce(induce, (Muon##title *) node);
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}
