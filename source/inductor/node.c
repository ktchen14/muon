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

/// @internal Called to continue into the @a node
static MuonNode *on_continue(induce_t *induce, MuonNode *node)
  __attribute__((nonnull));

/// @internal Called, on return from a node, to induce the type of the @a node
static const MuonType *on_return(induce_t *induce, MuonNode *node)
  __attribute__((nonnull));

const MuonType *induce_node(induce_t *induce, MuonNode *root) {
  assert(induce->scheme == NULL);
  induce->scheme = &(mu_scheme_t) { .induce = induce, .id = induce->type_number };

  assert(root->id < induce->node_length);

  MuonNode *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL) {
      node = node_continue(node, next);

      MuonNode *result;
      if ((result = on_continue(induce, node)) == NULL)
        return NULL;
      assert(result == node);
    }

    // Induce the type of the node
    const MuonType *type;
    if ((type = on_return(induce, node)) == NULL)
      return NULL;

    induce->result[node->id].source_type = type;
  } while ((node = node_return(node)) != NULL);

  return evince_type(induce, root);
}

__attribute__((nonnull)) static const MuonType *access_expr_return(
    induce_t *induce, MuonAccessExpr *expr) {
  const mu_variable_type_t *variable_type;
  if ((variable_type = mu_variable_type(induce)) == NULL)
    return NULL;

  const mu_core_t *core;
  if ((core = single_record_core(induce, expr->name)) == NULL)
    return NULL;
  assert(core->kind == MU_RECORD_CORE);

  const MuonType *record_argv[] = { &variable_type->as_type };
  const mu_core_type_t *record_type;
  if ((record_type = mu_core_type(induce, core, record_argv)) == NULL)
    return NULL;

  const MuonType *argv[] = { &record_type->as_type, &variable_type->as_type };
  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argv[0], argv[1])) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *boolean_expr_return(
    induce_t *induce, MuonBooleanExpr *expr) {
  const mu_core_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *cast_expr_return(
    induce_t *induce, MuonCastExpr *expr) {
  const MuonType *sign_type = evince_type(induce, &expr->sign->as_node);
  const MuonType *matter_type = evince_type(induce, &expr->matter->as_node);

  const mu_coercion_t *coercion;
  if ((coercion = ensure_coercion(induce, matter_type, sign_type)) == NULL)
    return NULL;
  assign_coercion_to_node(induce, &expr->matter->as_node, coercion, sign_type);

  return sign_type;
}

__attribute__((nonnull)) static const MuonType *integer_expr_return(
    induce_t *induce, MuonIntegerExpr *expr) {
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *invoke_expr_return(
    induce_t *induce, MuonInvokeExpr *expr) {
  const MuonType *operator_type, *argument_type;
  operator_type = evince_type(induce, &expr->operator->as_node);
  argument_type = evince_type(induce, &expr->argument->as_node);

  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;

  const MuonType *argv[] = { argument_type, &result->as_type };
  const mu_core_type_t *lambda_type;
  if ((lambda_type = mu_lambda_type(induce, argv[0], argv[1])) == NULL)
    return NULL;

  const MuonType *target = &lambda_type->as_type;
  const mu_coercion_t *coercion;
  if ((coercion = ensure_coercion(induce, operator_type, target)) == NULL)
    return NULL;
  assign_coercion_to_node(induce, &expr->operator->as_node, coercion, target);

  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *lambda_expr_return(
    induce_t *induce, MuonLambdaExpr *expr) {
  const MuonType *argument_type = evince_type(induce, &expr->argument->as_node);
  const MuonType *output_type = evince_type(induce, &expr->matter->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *name_expr_return(
    induce_t *induce, MuonNameExpr *expr) {
  MuonNode *target = detect_evince(induce->detect, &expr->as_node);
  assert(target != NULL);
  return evince_type(induce, target);
}

__attribute__((nonnull)) static const MuonType *native_expr_return(
    induce_t *induce, MuonNativeExpr *expr) {
  const mu_core_type_t *integer_type;
  if ((integer_type = mu_integer_type(induce)) == NULL)
    return NULL;

  const mu_core_type_t *vector_type;
  if ((vector_type = mu_vector_type(induce, &integer_type->as_type)) == NULL)
    return NULL;

  const MuonType *argument_type = &vector_type->as_type;
  const MuonType *output_type = &integer_type->as_type;
  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *record_expr_return(
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

__attribute__((nonnull)) static const MuonType *switch_case_return(
    induce_t *induce, MuonSwitchCase *node) {
  MuonNode *target;
  if ((target = detect_evince(induce->detect, &node->as_node)) == NULL)
    abort();
  const MuonType *case_type = evince_type(induce, target);
  assert(case_type->kind != MU_SCHEME_TYPE);

  const MuonType *expr_type = evince_type(induce, &node->expr->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, case_type, expr_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *switch_expr_return(
    induce_t *induce, MuonSwitchExpr *expr) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const MuonType *type = evince_type(induce, &expr->argv[i]->as_node);

    const mu_coercion_t *coercion;
    if ((coercion = ensure_coercion(induce, type, &result->as_type)) == NULL)
      return NULL;
    assign_coercion_to_node(induce, &expr->argv[i]->as_node, coercion, &result->as_type);
  }

  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *sequence_expr_return(
    induce_t *induce, MuonSequenceExpr *expr) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *vector_expr_return(
    induce_t *induce, MuonVectorExpr *expr) {
  const mu_variable_type_t *matter_type;
  if ((matter_type = mu_variable_type(induce)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const MuonType *type = evince_type(induce, &expr->argv[i]->as_node);

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

__attribute__((nonnull)) static const MuonType *boolean_sign_return(
    induce_t *induce, MuonBooleanSign *sign) {
  const mu_core_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *integer_sign_return(
    induce_t *induce, MuonIntegerSign *sign) {
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *lambda_sign_return(
    induce_t *induce, MuonLambdaSign *sign) {
  const MuonType *argument_type = evince_type(induce, &sign->argument->as_node);
  const MuonType *output_type = evince_type(induce, &sign->output->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *name_sign_return(
    induce_t *induce, MuonNameSign *sign) {
  MuonNode *target = detect_evince(induce->detect, &sign->as_node);
  assert(target != NULL);
  return evince_type(induce, target);
}

__attribute__((nonnull)) static const MuonType *record_sign_return(
    induce_t *induce, MuonRecordSign *sign) {
  assert(0);
}

__attribute__((nonnull)) static const MuonType *vector_sign_return(
    induce_t *induce, MuonVectorSign *sign) {
  const MuonType *matter = evince_type(induce, &sign->matter->as_node);

  const mu_core_type_t *result;
  if ((result = mu_vector_type(induce, matter)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *coercion_stmt_return(
    induce_t *induce, MuonCoercionStmt *stmt) {
  const MuonType *source_type = evince_type(induce, &stmt->source->as_node);
  const MuonType *target_type = evince_type(induce, &stmt->target->as_node);
  const MuonType *expr_type = evince_type(induce, &stmt->expr->as_node);

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

__attribute__((nonnull)) static const MuonType *datatype_option_return(
    induce_t *induce, MuonDatatypeOption *option) {
  const mu_core_t *core = induce->datatype_core;
  assert(core != NULL);

  const mu_core_type_t *result;
  if ((result = mu_core_type(induce, core, NULL)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static MuonNode *datatype_stmt_continue(
    induce_t *induce, MuonDatatypeStmt *stmt) {
  const mu_core_t *core;
  if ((core = mu_simple_core(induce, stmt->name)) == NULL)
    return NULL;
  induce->core[induce->core_length++] = core;
  induce->datatype_core = core;

  return &stmt->as_node;
}

__attribute__((nonnull)) static const MuonType *datatype_stmt_return(
    induce_t *induce, MuonDatatypeStmt *stmt) {
  // TODO: No arguments supported for now
  const mu_core_t *core = induce->datatype_core;
  const mu_core_type_t *result;
  if ((result = mu_core_type(induce, core, NULL)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static MuonNode *define_stmt_continue(
    induce_t *induce, MuonDefineStmt *stmt) {
  mu_scheme_t *scheme;
  if ((scheme = mu_scheme(induce->scheme)) == NULL)
    return NULL;
  induce->scheme = scheme;

  return &stmt->as_node;
}

__attribute__((nonnull)) static const MuonType *define_stmt_return(
    induce_t *induce, MuonDefineStmt *stmt) {
  const MuonType *expr_type = evince_type(induce, &stmt->expr->as_node);

  const MuonType *result;
  if ((result = generalize_type(induce, expr_type)) == NULL)
    return NULL;

  mu_scheme_t *parent = induce->scheme->parent;
  free(induce->scheme);
  induce->scheme = parent;

  return result;
}

__attribute__((nonnull)) static const MuonType *record_view_return(
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

__attribute__((nonnull)) static const MuonType *variable_view_return(
    induce_t *induce, MuonVariableView *view) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const MuonType *expr_member_return(
    induce_t *induce, MuonExprMember *member) {
  return evince_type(induce, &member->expr->as_node);
}

__attribute__((nonnull)) static const MuonType *view_member_return(
    induce_t *induce, MuonViewMember *member) {
  return evince_type(induce, &member->view->as_node);
}

static MuonNode *on_continue(induce_t *induce, MuonNode *node) {
  switch ON_ABSTRACT_OBJECT(node) {
    case IS_CONCRETE_NODE(MuonDatatypeStmt *nominate(datatype_stmt))
      return datatype_stmt_continue(induce, datatype_stmt);

    case IS_CONCRETE_NODE(MuonDefineStmt *nominate(define_stmt))
      return define_stmt_continue(induce, define_stmt);

    default: return node;
  }
  __builtin_unreachable();
}

static const MuonType *on_return(induce_t *induce, MuonNode *node) {
  switch ON_ABSTRACT_OBJECT(node) {
#define MUON_EMIT(lower, upper, title) \
    case MUON_##upper##_NODE: \
      return lower##_return(induce, (Muon##title *) node);
    MU_EACH_NODE_KIND(MUON_EMIT)
#undef MUON_EMIT
  }
  __builtin_unreachable();
}
