#include "induce.h"

#include "detect.h"
#include "../stator.h"
#include "../status.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

_Thread_local induce_t *debug_induce;

const type_t *boolean_type(induce_t *induce) {
  type_t *result;
  if ((result = malloc(sizeof(type_t))) == NULL)
    return NULL;
  *result = (type_t) { .kind = SIMPLE_TYPE, .core = induce->boolean_core, .level = 0 };
  return result;
}

const type_t *integer_type(induce_t *induce) {
  type_t *result;
  if ((result = malloc(sizeof(type_t))) == NULL)
    return NULL;
  *result = (type_t) { .kind = SIMPLE_TYPE, .core = induce->integer_core, .level = 0 };
  return result;
}

const type_t *lambda_type(induce_t *induce, const type_t *argument, const type_t *output) {
  size_t size = struct_size(type_t, argv, 2);
  assert(induce->lambda_core->argc == 2);


  type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (type_t) {
    .kind = SIMPLE_TYPE,
    .core = induce->lambda_core,
    .level = argument->level > output->level ? argument->level : output->level,
  };

  result->argv[0] = argument;
  result->argv[1] = output;

  return result;
}

const type_t *record_type(mu_engine_t *engine, size_t argc, const type_member_t[static argc]);

const type_t *variable_type(mu_engine_t *engine, size_t level) {
  type_t *result;
  if ((result = malloc(sizeof(type_t))) == NULL)
    return NULL;
  *result = (type_t) { .kind = VARIABLE_TYPE, .level = level };
  return result;
}

const type_t *vector_type(induce_t *induce, const type_t *matter) {
  size_t size = struct_size(type_t, argv, 1);
  assert(induce->vector_core->argc == 1);

  type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (type_t) {
    .kind = SIMPLE_TYPE,
    .core = induce->vector_core,
    .level = matter->level,
  };
  result->argv[0] = matter;

  return result;
}

/// Register @a a <: @a b in the @a induce engine
static const mu_type_t *append(
    induce_t *induce, const mu_type_t *restrict a, const mu_type_t *restrict b)
  __attribute__((nonnull));

/**
 * @brief Restrict type @a a to be a subtype of @a b in the @a induce engine
 *
 * On allocation failure, @c errno is set by the allocator. This function can't
 * fail otherwise. The behavior is undefined if:
 *
 * - @a induce, @a a, or @a b is @c NULL
 * - @a a or @a b isn't in the same zone as that of the @a induce engine
 *
 * @param induce the induce engine to restrict @a a and @a b within
 * @param a the type to restrict to a subtype of @a b
 * @param b the type to restrict to a supertype of @a a
 */
static induce_t *restrict_type(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b)
  __attribute__((nonnull));

/// Induce the type of the abstract @a node with the @a induce engine
static const mu_type_t *node_induce(const mu_node_t *node, induce_t *induce)
  __attribute__((nonnull));

induce_t *induce_initialize(
    induce_t *induce,
    mu_engine_t *engine,
    mu_status_t *status,
    const detect_t *detect) {
  assert(detect_result(detect)->engine == engine);

  size_t node_length = engine->node_number;

  const mu_type_t **node_to_type;
  if ((node_to_type = malloc(sizeof(const mu_type_t *[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; node_to_type[i++] = NULL);

  size_t sub_volume = 1;
  induce_sub_t *sub_data;
  if ((sub_data = malloc(sizeof(induce_sub_t[sub_volume]))) == NULL)
    return NULL;
  for (size_t i = 0; i < sub_volume; sub_data[i++] = (induce_sub_t) {0});

  mu_core_t *boolean_core;
  if ((boolean_core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *boolean_core = (mu_core_t) { .kind = MU_BOOLEAN_HEAD };

  mu_core_t *integer_core;
  if ((integer_core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *integer_core = (mu_core_t) { .kind = MU_INTEGER_HEAD };

  size_t size;

  mu_core_t *lambda_core;
  size = struct_size(mu_core_t, variance, 2);
  if ((lambda_core = malloc(size)) == NULL)
    return NULL;
  *lambda_core = (mu_core_t) { .kind = MU_LAMBDA_HEAD, .argc = 2 };
  lambda_core->variance[0] = MU_CONTRAVARIANCE;
  lambda_core->variance[1] = MU_COVARIANCE;

  mu_core_t *vector_core;
  size = struct_size(mu_core_t, variance, 1);
  if ((vector_core = malloc(size)) == NULL)
    return NULL;
  *vector_core = (mu_core_t) { .kind = MU_VECTOR_HEAD, .argc = 1 };
  vector_core->variance[1] = MU_COVARIANCE;

  *induce = (induce_t) {
    .engine = engine,
    .status = status,
    .detect = detect_result(detect),
    .node_length = node_length,
    .node_to_type = node_to_type,
    .sub_volume = sub_volume,
    .sub_data = sub_data,

    .boolean_core = boolean_core,
    .integer_core = integer_core,
    .lambda_core = lambda_core,
    .vector_core = vector_core,
  };
  return induce;
}

const mu_type_t *induce_node(induce_t *induce, const mu_node_t *root) {
  assert(root->as_stator.id < induce->node_length);

  if (induce->node_to_type[root->as_stator.id] != NULL)
    return induce->node_to_type[root->as_stator.id];

  const mu_node_t *node = root, *next;
  do {
    const detect_result_t *detect = induce->detect;
    while ((next = detect_at(detect, node, node_cursor(node)->i++)) != NULL) {
      assert(next->as_stator.id < induce->node_length);

      if (induce->node_to_type[next->as_stator.id] != NULL)
        continue;
      node = node_continue(node, next);
    }

    // Induce the type of the node
    const mu_type_t *type;
    if ((type = node_induce(node, induce)) == NULL)
      return NULL;
    induce->node_to_type[node->as_stator.id] = type;
  } while ((node = node_return(node)) != NULL);

  return induce_evince(induce, root);
}

static induce_t *restrict_type(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b) {
  // If we don't have to continue into a or b, then just return
  for (size_t i = 0; i < induce->sub_length; i++) {
    if (induce->sub_data[i].lower == a && induce->sub_data[i].upper == b)
      return induce;
  }

  if (a->kind == MU_BOOLEAN_TYPE && b->kind == MU_BOOLEAN_TYPE) {
    append(induce, a, b);
    return induce;
  }

  if (a->kind == MU_INTEGER_TYPE && b->kind == MU_INTEGER_TYPE) {
    append(induce, a, b);
    return induce;
  }

  if (a->kind == MU_LAMBDA_TYPE && b->kind == MU_LAMBDA_TYPE) {
    const mu_lambda_type_t *ra = (const mu_lambda_type_t *) a;
    const mu_lambda_type_t *rb = (const mu_lambda_type_t *) b;

    if (restrict_type(induce, rb->argument, ra->argument) == NULL)
      return NULL;
    if (restrict_type(induce, ra->output, rb->output) == NULL)
      return NULL;
    append(induce, a, b);
    return induce;
  }

  if (a->kind == MU_RECORD_TYPE && b->kind == MU_RECORD_TYPE) {
    const mu_record_type_t *ra = (const mu_record_type_t *) a;
    const mu_record_type_t *rb = (const mu_record_type_t *) b;

    for (size_t j = 0; j < rb->argc; j++) {
      for (size_t i = 0; i < ra->argc; i++) {
        if (ra->argv[i].name == rb->argv[j].name) {
          if (restrict_type(induce, ra->argv[i].type, rb->argv[j].type) == NULL)
            return NULL;
          goto next;
        }
      }

      fprintf(stderr, "Type mismatch\n");
      abort();

    next:;
    }

    append(induce, a, b);
    return induce;
  }

  if (a->kind == MU_VECTOR_TYPE && b->kind == MU_VECTOR_TYPE) {
    const mu_vector_type_t *ra = (const mu_vector_type_t *) a;
    const mu_vector_type_t *rb = (const mu_vector_type_t *) b;

    if (restrict_type(induce, ra->matter, rb->matter) == NULL)
      return NULL;

    append(induce, a, b);
    return induce;
  }

  const mu_variable_type_t *va = mu_type_cast(a, va);
  const mu_variable_type_t *vb = mu_type_cast(b, vb);

  if (va == NULL && vb == NULL) {
    fprintf(stderr, "Type mismatch\n");
    abort();
  }

  if (va != NULL) {
    for (size_t i = 0; i < induce->sub_length; i++) {
      if (induce->sub_data[i].upper != &va->as_type)
        continue;
      if (restrict_type(induce, induce->sub_data[i].lower, &vb->as_type) == NULL)
        return NULL;
    }
  }

  if (vb != NULL) {
    for (size_t j = 0; j < induce->sub_length; j++) {
      if (induce->sub_data[j].lower != &vb->as_type)
        continue;
      if (restrict_type(induce, &va->as_type, induce->sub_data[j].upper) == NULL)
        return NULL;
    }
  }

  append(induce, a, b);
  return induce;
}

static const mu_type_t *append(
    induce_t *induce, const mu_type_t *restrict a, const mu_type_t *restrict b) {
  for (size_t i = 0; i < induce->sub_length; i++) {
    induce_sub_t sub = induce->sub_data[i];
    if (sub.lower == a && sub.upper == b)
      return a;
  }

  if (induce->sub_length >= induce->sub_volume) {
    size_t volume = induce->sub_volume;
    if (rare(__builtin_mul_overflow(volume, 2, &volume)))
      return errno = ENOMEM, NULL;

    size_t size;
    if (rare(__builtin_mul_overflow(volume, sizeof(induce_sub_t), &size)))
      return errno = ENOMEM, NULL;

    induce_sub_t *sub_data = induce->sub_data;
    if ((sub_data = realloc(sub_data, size)) == NULL)
      return NULL;
    for (size_t i = induce->sub_volume; i < volume; i++)
      sub_data[i] = (induce_sub_t) {0};

    induce->sub_volume = volume;
    induce->sub_data = sub_data;
  }

  induce->sub_data[induce->sub_length++] = (induce_sub_t) {
    .lower = a, .upper = b };
  return b;
}

// ---------------------------------- Expr -------------------------------- {{{1

__attribute__((nonnull)) static const type_t *access_expr_induce(
    const mu_access_expr_t *expr, induce_t *induce, size_t level) {
  mu_engine_t *engine = induce->engine;

  const type_t *result;
  if ((result = variable_type(engine, level)) == NULL)
    return NULL;

  const type_t *record_ty;
  const type_member_t argv[] = {
    { .name = expr->name, .type = result }
  };
  if ((record_ty = record_type(engine, 1, argv)) == NULL)
    return NULL;

  const mu_type_t *matter_type = induce_evince(induce, &expr->matter->as_node);
  if (restrict_type(induce, matter_type, record_ty) == NULL)
    return NULL;
  return result;
}

__attribute__((nonnull)) static const type_t *boolean_expr_induce(
    const mu_boolean_expr_t *expr, induce_t *induce) {
  return boolean_type(induce->engine);
}

__attribute__((nonnull)) static const type_t *integer_expr_induce(
    const mu_integer_expr_t *expr, induce_t *induce) {
  return integer_type(induce->engine);
}

__attribute__((nonnull)) static const type_t *invoke_expr_induce(
    const mu_invoke_expr_t *expr, induce_t *induce, size_t level) {
  const type_t *lambda = induce_evince(induce, &expr->lambda->as_node);
  const type_t *matter = induce_evince(induce, &expr->matter->as_node);

  const type_t *result;
  if ((result = variable_type(induce->engine, level)) == NULL)
    return NULL;

  const type_t *lambda_ty;
  if ((lambda_ty = lambda_type(induce->engine, matter, result)) == NULL)
    return NULL;

  if (restrict_type(induce, lambda, lambda_ty) == NULL)
    return NULL;
  return result;
}

__attribute__((nonnull)) static const mu_type_t *lambda_expr_induce(
    const mu_lambda_expr_t *expr, induce_t *induce) {
  const mu_type_t *argument = induce_evince(induce, &expr->argument->as_node);
  const mu_type_t *output = induce_evince(induce, &expr->matter->as_node);

  const mu_lambda_type_t *result;
  if ((result = mu_lambda_type(induce->engine, argument, output)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const type_t *name_expr_induce(
    const mu_name_expr_t *expr, induce_t *induce, size_t level) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &expr->as_node)) != NULL)
    return induce_evince(induce, target);
  return variable_type(induce->engine, level);
}

__attribute__((nonnull)) static const mu_type_t *record_expr_induce(
    const mu_record_expr_t *expr, induce_t *induce) {
  mu_engine_t *engine = induce->engine;

  mu_record_type_t *allocation;
  if ((allocation = record_type_allocate(engine, expr->argc)) == NULL)
    return NULL;

  size_t i = 0, j = expr->argc;
  for (size_t k = 0; k < expr->argc; k++) {
    const mu_name_t *member_name = expr->argv[k].name;
    const mu_expr_t *member_expr = expr->argv[k].expr;

    mu_type_member_t member = {
      .name = member_name, .type = induce_evince(induce, &member_expr->as_node),
    };
    allocation->argv[member.name == NULL ? i++ : --j] = member;
  }
  assert(i == j);
  qsort(&allocation->argv[j], expr->argc - j, sizeof(mu_expr_member_t),
      type_member_cmp);
  // TODO: check for duplicates
  return &record_type_activate(allocation)->as_type;
}

__attribute__((nonnull)) static const mu_type_t *sequence_expr_induce(
    const mu_sequence_expr_t *expr, induce_t *induce) {
  return induce_evince(induce, &expr->output->as_node);
}

__attribute__((nonnull)) static const type_t *vector_expr_induce(
    const mu_vector_expr_t *expr, induce_t *induce, size_t level) {
  const type_t *matter_type;
  if ((matter_type = variable_type(induce->engine, level)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = induce_evince(induce, &expr->argv[i]->as_node);
    if (restrict_type(induce, type, &matter_type->as_type) == NULL)
      return NULL;
  }

  return vector_type(induce->engine, matter_type);
}

__attribute__((nonnull)) static const type_t *zero_expr_induce(
    const mu_zero_expr_t *expr, induce_t *induce, size_t level) {
  return variable_type(induce->engine, level);
}

// ---------------------------------- Sign -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *boolean_sign_induce(
    const mu_boolean_sign_t *sign, induce_t *induce) {
  const mu_boolean_type_t *result;
  if ((result = mu_boolean_type(induce->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_sign_induce(
    const mu_integer_sign_t *sign, induce_t *induce) {
  const mu_integer_type_t *result;
  if ((result = mu_integer_type(induce->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *name_sign_induce(
    const mu_name_sign_t *sign, induce_t *induce) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &sign->as_node)) != NULL)
    return induce_evince(induce, target);

  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *record_sign_induce(
    const mu_record_sign_t *sign, induce_t *induce) {
  assert(0);
}

__attribute__((nonnull)) static const mu_type_t *variable_sign_induce(
    const mu_variable_sign_t *sign, induce_t *induce) {
  assert(0);
}

__attribute__((nonnull)) static const mu_type_t *vector_sign_induce(
    const mu_vector_sign_t *sign, induce_t *induce) {
  const mu_type_t *matter = induce_evince(induce, &sign->matter->as_node);

  const mu_vector_type_t *result;
  if ((result = mu_vector_type(induce->engine, matter)) == NULL)
    return NULL;
  return &result->as_type;
}

// ---------------------------------- Stmt -------------------------------- {{{1

__attribute__((nonnull, pure)) static const mu_type_t *define_stmt_induce(
    const mu_define_stmt_t *stmt, induce_t *induce) {
  return induce_evince(induce, &stmt->expr->as_node);
}

__attribute__((nonnull)) static const mu_type_t *type_stmt_induce(
    const mu_type_stmt_t *stmt, induce_t *induce) {
  assert(0);
}

// ---------------------------------- View -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *variable_view_induce(
    const mu_variable_view_t *view, induce_t *induce) {
  const mu_variable_type_t *result;
  if ((result = mu_variable_type(induce->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

// -------------------------------- Abstract ------------------------------ {{{1

static const mu_type_t *node_induce(const mu_node_t *node, induce_t *induce) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_NODE: \
      return lower##_induce((const mu_##lower##_t *) node, induce);
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

// vim: set foldmethod=marker:
