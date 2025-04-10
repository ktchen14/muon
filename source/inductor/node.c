#include "../common.h"
#include "../stator.h"
#include "coercion.h"
#include "core.h"
#include "detect.h"
#include "induce.h"
#include "type.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static const mu_type_t *node_induce(induce_t *induce, const mu_node_t *node);

const mu_type_t *induce_node(induce_t *induce, const mu_node_t *root) {
  assert(induce->scheme == NULL);
  induce->scheme = &(mu_scheme_t) { .induce = induce };

  assert(root->id < induce->node_length);

  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL) {
      node = node_continue(node, next);

      const mu_datatype_stmt_t *datatype_stmt;
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

    induce->node_to_type[node->id] = type;
  } while ((node = node_return(node)) != NULL);

  return evince_type(induce, root);
}

__attribute__((nonnull)) static const mu_type_t *access_expr_induce(
    induce_t *induce, const mu_access_expr_t *expr) {
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
    induce_t *induce, const mu_boolean_expr_t *expr) {
  const mu_core_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *cast_expr_induce(
    induce_t *induce, const mu_cast_expr_t *expr) {
  const mu_type_t *sign_type = evince_type(induce, &expr->sign->as_node);
  const mu_type_t *matter_type = evince_type(induce, &expr->matter->as_node);

  const mu_coercion_t *coercion;
  if ((coercion = ensure_coercion(induce, matter_type, sign_type)) == NULL)
    return NULL;
  assign_coercion(induce, &expr->matter->as_node, coercion, sign_type);

  return sign_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_expr_induce(
    induce_t *induce, const mu_integer_expr_t *expr) {
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *invoke_expr_induce(
    induce_t *induce, const mu_invoke_expr_t *expr) {
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
  assign_coercion(induce, &expr->operator->as_node, coercion, target);

  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *lambda_expr_induce(
    induce_t *induce, const mu_lambda_expr_t *expr) {
  const mu_type_t *argument_type = evince_type(induce, &expr->argument->as_node);
  const mu_type_t *output_type = evince_type(induce, &expr->matter->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *name_expr_induce(
    induce_t *induce, const mu_name_expr_t *expr) {
  const mu_node_t *target = detect_evince(induce->detect, &expr->as_node);
  assert(target != NULL);

  const mu_type_t *result = evince_type(induce, target);
  if (result->kind != MU_SCHEME_TYPE)
    return result;

  const mu_scheme_type_t *scheme_type = (const mu_scheme_type_t *) result;

  // Instantiate the polymorphic type
  result = instantiate_scheme(induce, scheme_type);
  return result;
}

__attribute__((nonnull)) static const mu_type_t *native_expr_induce(
    induce_t *induce, const mu_native_expr_t *expr) {
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
    induce_t *induce, const mu_record_expr_t *expr) {
  mu_core_t *core_allocation;
  if ((core_allocation = record_core_allocate(induce, expr->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_name_t *name = expr->argv[i]->name;

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
    induce_t *induce, const mu_switch_case_t *node) {
  const mu_node_t *target;
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
    induce_t *induce, const mu_switch_expr_t *expr) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = evince_type(induce, &expr->argv[i]->as_node);

    const mu_coercion_t *coercion;
    if ((coercion = ensure_coercion(induce, type, &result->as_type)) == NULL)
      return NULL;
    assign_coercion(induce, &expr->argv[i]->as_node, coercion, &result->as_type);
  }

  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *sequence_expr_induce(
    induce_t *induce, const mu_sequence_expr_t *expr) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *vector_expr_induce(
    induce_t *induce, const mu_vector_expr_t *expr) {
  const mu_variable_type_t *matter_type;
  if ((matter_type = mu_variable_type(induce)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = evince_type(induce, &expr->argv[i]->as_node);

    const mu_coercion_t *coercion;
    if ((coercion = ensure_coercion(induce, type, &matter_type->as_type)) == NULL)
      return NULL;
    assign_coercion(induce, &expr->argv[i]->as_node, coercion, &matter_type->as_type);
  }

  const mu_core_type_t *result;
  if ((result = mu_vector_type(induce, &matter_type->as_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *zero_expr_induce(
    induce_t *induce, const mu_zero_expr_t *expr) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *boolean_sign_induce(
    induce_t *induce, const mu_boolean_sign_t *sign) {
  const mu_core_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_sign_induce(
    induce_t *induce, const mu_integer_sign_t *sign) {
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *lambda_sign_induce(
    induce_t *induce, const mu_lambda_sign_t *sign) {
  const mu_type_t *argument_type = evince_type(induce, &sign->argument->as_node);
  const mu_type_t *output_type = evince_type(induce, &sign->output->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *name_sign_induce(
    induce_t *induce, const mu_name_sign_t *sign) {
  const mu_node_t *target = detect_evince(induce->detect, &sign->as_node);
  assert(target != NULL);
  return evince_type(induce, target);
}

__attribute__((nonnull)) static const mu_type_t *record_sign_induce(
    induce_t *induce, const mu_record_sign_t *sign) {
  assert(0);
}

__attribute__((nonnull)) static const mu_type_t *vector_sign_induce(
    induce_t *induce, const mu_vector_sign_t *sign) {
  const mu_type_t *matter = evince_type(induce, &sign->matter->as_node);

  const mu_core_type_t *result;
  if ((result = mu_vector_type(induce, matter)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *coercion_stmt_induce(
    induce_t *induce, const mu_coercion_stmt_t *stmt) {
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
  assign_coercion(induce, &stmt->expr->as_node, coercion, &lambda_type->as_type);

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
    induce_t *induce, const mu_datatype_option_t *option) {
  const mu_core_t *core = induce->datatype_core;
  assert(core != NULL);

  const mu_core_type_t *result;
  if ((result = mu_core_type(induce, core, NULL)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *datatype_stmt_induce(
    induce_t *induce, const mu_datatype_stmt_t *stmt) {
  // TODO: No arguments supported for now
  const mu_core_t *core = induce->datatype_core;
  const mu_core_type_t *result;
  if ((result = mu_core_type(induce, core, NULL)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *define_stmt_induce(
    induce_t *induce, const mu_define_stmt_t *stmt) {
  const mu_type_t *expr_type = evince_type(induce, &stmt->expr->as_node);
  return generalize_type(induce, expr_type, induce->scheme);
}

__attribute__((nonnull)) static const mu_type_t *record_view_induce(
    induce_t *induce, const mu_record_view_t *view) {
  mu_core_t *core_allocation;
  if ((core_allocation = record_core_allocate(induce, view->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < view->argc; i++) {
    const mu_name_t *name = view->argv[i]->name;

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
    induce_t *induce, const mu_variable_view_t *view) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *expr_member_induce(
    induce_t *induce, const mu_expr_member_t *member) {
  return evince_type(induce, &member->expr->as_node);
}

__attribute__((nonnull)) static const mu_type_t *view_member_induce(
    induce_t *induce, const mu_view_member_t *member) {
  return evince_type(induce, &member->view->as_node);
}

static const mu_type_t *node_induce(induce_t *induce, const mu_node_t *node) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_NODE: \
      return lower##_induce(induce, (const mu_##lower##_t *) node);
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}
