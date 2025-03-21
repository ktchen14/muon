#include "induce.h"

#include "detect.h"
#include "../stator.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#define evince induce_reveal

static const mu_type_t *node_induce(const mu_node_t *node, induce_t *induce, open_scheme_t *scheme);

const mu_type_t *induce_node(induce_t *induce, const mu_node_t *root) {
  assert(root->id < induce->node_length);

  open_scheme_t root_scheme = { .induce = induce, .node = root };
  open_scheme_t *scheme = &root_scheme;

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

      scheme = open_scheme(scheme, node);
    }

    // Induce the type of the node
    const mu_type_t *type;
    if ((type = node_induce(node, induce, scheme)) == NULL)
      return NULL;

    if (node->kind == MU_DEFINE_STMT_NODE) {
      open_scheme_t *parent = scheme->parent;
      free(scheme);
      scheme = parent;
    }

    induce->node_to_type[node->id] = type;
  } while ((node = node_return(node)) != NULL);

  return induce_reveal(induce, root);
}

// ---------------------------------- Expr -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *access_expr_induce(
    const mu_access_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;

  const mu_core_t *core;
  if ((core = single_record_core(induce, expr->name)) == NULL)
    return NULL;
  assert(core->kind == MU_RECORD_CORE);

  mu_core_type_t *allocation;
  if ((allocation = core_type_allocate(induce, core)) == NULL)
    return NULL;
  allocation->argv[0] = &result->as_type;

  const mu_core_type_t *record_type;
  if (rare((record_type = core_type_activate(allocation)) == NULL))
    return NULL;
  induce->aux[expr->as_node.id] = &record_type->as_type;

  const mu_type_t *matter_type = induce_reveal(induce, &expr->matter->as_node);
  if (restrict_type(induce, matter_type, &record_type->as_type) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *boolean_expr_induce(
    const mu_boolean_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_expr_induce(
    const mu_integer_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *invoke_expr_induce(
    const mu_invoke_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_type_t *operator_type = evince(induce, &expr->operator->as_node);
  const mu_type_t *argument_type = evince(induce, &expr->argument->as_node);

  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;

  const mu_core_type_t *lambda_type;
  if ((lambda_type = mu_lambda_type(induce, argument_type, &result->as_type)) == NULL)
    return NULL;
  induce->aux[expr->as_node.id] = &lambda_type->as_type;

  if (restrict_type(induce, operator_type, &lambda_type->as_type) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *lambda_expr_induce(
    const mu_lambda_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_type_t *argument_type = evince(induce, &expr->argument->as_node);
  const mu_type_t *output_type = evince(induce, &expr->matter->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *name_expr_induce(
    const mu_name_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &expr->as_node)) == NULL) {
    abort();
    const mu_variable_type_t *result;
    if ((result = variable_type(induce, scheme)) == NULL)
      return NULL;
    return &result->as_type;
  }

  const mu_type_t *result = induce_reveal(induce, target);
  if (result->kind != MU_SCHEME_TYPE)
    return result;

  // Instantiate the polymorphic type
  /* result = instantiate_scheme(induce, result, scheme); */
  return result;
}

__attribute__((nonnull)) static const mu_type_t *native_expr_induce(
    const mu_native_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
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
    const mu_record_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
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
    type_allocation->argv[i] = induce_reveal(induce, &expr->argv[i]->as_node);

  const mu_core_type_t *result;
  if (rare((result = core_type_activate(type_allocation)) == NULL))
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *switch_case_induce(
    const mu_switch_case_t *node, induce_t *induce, open_scheme_t *scheme) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &node->as_node)) == NULL)
    abort();
  const mu_type_t *case_type = induce_reveal(induce, target);
  assert(case_type->kind != MU_SCHEME_TYPE);

  const mu_type_t *expr_type = evince(induce, &node->expr->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, case_type, expr_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *switch_expr_induce(
    const mu_switch_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = induce_reveal(induce, &expr->argv[i]->as_node);
    if (restrict_type(induce, type, &result->as_type) == NULL)
      return NULL;
  }

  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *sequence_expr_induce(
    const mu_sequence_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *vector_expr_induce(
    const mu_vector_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *matter_type;
  if ((matter_type = variable_type(induce, scheme)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = induce_reveal(induce, &expr->argv[i]->as_node);
    if (restrict_type(induce, type, &matter_type->as_type) == NULL)
      return NULL;
  }

  const mu_core_type_t *result;
  if ((result = mu_vector_type(induce, &matter_type->as_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *zero_expr_induce(
    const mu_zero_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;
  return &result->as_type;
}

// ---------------------------------- Sign -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *boolean_sign_induce(
    const mu_boolean_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_sign_induce(
    const mu_integer_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *name_sign_induce(
    const mu_name_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &sign->as_node)) != NULL)
    return induce_reveal(induce, target);

  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *record_sign_induce(
    const mu_record_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  assert(0);
}

__attribute__((nonnull)) static const mu_type_t *vector_sign_induce(
    const mu_vector_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const mu_type_t *matter = induce_reveal(induce, &sign->matter->as_node);

  const mu_core_type_t *result;
  if ((result = mu_vector_type(induce, matter)) == NULL)
    return NULL;
  return &result->as_type;
}

// ---------------------------------- Stmt -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *datatype_option_induce(
    const mu_datatype_option_t *option, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_t *core = induce->datatype_core;
  assert(core != NULL);

  const mu_core_type_t *result;
  if ((result = mu_core_type(induce, core, NULL)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *datatype_stmt_induce(
    const mu_datatype_stmt_t *stmt, induce_t *induce, open_scheme_t *scheme) {
  // TODO: Fake this
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull, pure)) static const mu_type_t *define_stmt_induce(
    const mu_define_stmt_t *stmt, induce_t *induce, open_scheme_t *scheme) {
  assert(scheme->node == &stmt->as_node);

  const mu_type_t *expr_type = induce_reveal(induce, &stmt->expr->as_node);
  return expr_type;
  /* mark_type(induce, expr_type, 0, scheme->rank); */

  size_t polymorphic_length = 0;
  mu_variable_type_t *polymorphic = NULL;

  mu_variable_type_t *type = scheme->link;
  while (type != NULL) {
    assert(type->rank == scheme->rank);

    mu_variable_type_t *next = type->scheme_next;

    /* Does a type have to be both positively reachable and negatively reachable
     * from the type of the defined expr to be polymorphic? */
    /* What is a polymorphic type polymorphic to? Just this type scheme? Or all
     * type schemes above this? Or all type scheme below this? */

    /*
     * Not sure if this is true, but here are some thoughts:
     *
     * A variable type must be constrained somehow to be polymorphically useful.
     * If we have:
     *   foo :: a
     * Then, while theoretically foo is polymorphic, it's not any more useful
     * than:
     *   foo :: ⊥
     *
     * Similarly, this function:
     *   bar :: a -> ()
     * While theoretically polymorphic, is no more useful than:
     *   bar :: ⊤ -> ()
     *
     * A variable can be constrained by either appearing both positively and
     * negatively, being constrained by bounds, or (in the future) being
     * constrained by kind. For now, just do this:
     */
    if (type->positively_reachable && type->negatively_reachable) {
      type->scheme_next = polymorphic;
      polymorphic = type;
      type->rank = 0;
      polymorphic_length++;
    } else {
      type->scheme_next = scheme->parent->link;
      scheme->parent->link = type;
      type->rank--;
    }

    type = next;
  }

  if (polymorphic_length == 0)
    return expr_type;
  return expr_type;

  /* mu_scheme_type_t *allocation; */
  /* if ((allocation = scheme_type_allocate(induce, polymorphic_length)) == NULL) */
  /*   return NULL; */

  /* size_t i = 0; */
  /* for (mu_variable_type_t *type = polymorphic; type != NULL; type = type->scheme_next) { */
  /*   allocation->argv[i++] = type; */
  /*   type->polymorphic_to = allocation; */
  /* } */

  /* const mu_scheme_type_t *result; */
  /* if (rare((result = scheme_type_activate(allocation, expr_type)) == NULL)) */
  /*   return NULL; */
  /* return &result->as_type; */
}

// ---------------------------------- View -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *variable_view_induce(
    const mu_variable_view_t *view, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;
  return &result->as_type;
}

// -------------------------------- Abstract ------------------------------ {{{1

__attribute__((nonnull)) static const mu_type_t *expr_member_induce(
    const mu_expr_member_t *member, induce_t *induce, open_scheme_t *scheme) {
  return induce_reveal(induce, &member->expr->as_node);
}

static const mu_type_t *node_induce(const mu_node_t *node, induce_t *induce, open_scheme_t *scheme) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_NODE: \
      return lower##_induce((const mu_##lower##_t *) node, induce, scheme);
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}
