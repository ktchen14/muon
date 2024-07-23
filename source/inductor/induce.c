#include "induce.h"

#include "detect.h"
#include "../stator.h"
#include "../status.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define PROBLEM 0
#define RETURN 1
#define TO_CONTINUE 2


static _Thread_local struct {
  const mu_type_t *a;
  const mu_type_t *b;
} continue_into;

static int CONTINUE(const mu_type_t *from_a, const mu_type_t *from_b, const mu_type_t *a, const mu_type_t *b) {
  mu_type_debug(a);
  fprintf(stderr, " <: ");
  mu_type_debug(b);
  fprintf(stderr, "\n");

  if (a == b)
    return RETURN;

  if (a->kind != MU_VARIABLE_TYPE && b->kind != MU_VARIABLE_TYPE && a->kind != b->kind)
    assert(0);
  continue_into.a = a;
  continue_into.b = b;

  return TO_CONTINUE;
}

#define CONTINUE(...) ({ \
    fprintf(stderr, "In %s: ", __func__); \
    CONTINUE(__VA_ARGS__); \
  })

/**
 * @brief Constrain type @a a to be a subtype of type @a b in the @a induce
 *   engine
 *
 * On allocation failure, @c errno is set by the allocator. This function can't
 * fail otherwise. The behavior is undefined if:
 *
 * - @a induce, @a a, or @a b is @c NULL
 * - @a a or @a b isn't in the same zone as that of the @a induce engine
 *
 * @param induce the induce engine
 * @param a the type to subsume to @a b
 * @param b the type to subsume to @a a
 * @return a type equivalent to both @a a and @a b on success; otherwise @c NULL
 */
static const mu_type_t *subsume(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b)
  __attribute__((nonnull));

static const mu_type_t *append(
    induce_t *induce,
    const mu_type_t *restrict lower,
    const mu_type_t *restrict upper)
  __attribute__((nonnull));

/// Induce the type of the abstract @a node with the @a induce engine
static const mu_type_t *node_induce(const mu_node_t *node, induce_t *induce)
  __attribute__((nonnull));

static int type_subsume(
    const mu_type_t *restrict a,
    const mu_type_t *restrict b,
    induce_t *induce);

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

  *induce = (induce_t) {
    .engine = engine,
    .status = status,
    .detect = detect_result(detect),
    .node_length = node_length,
    .node_to_type = node_to_type,
    .sub_volume = sub_volume,
    .sub_data = sub_data,
  };
  return induce;
}

__attribute__((nonnull, pure, unused))
static inline const mu_type_t *record_type_get_name(
    const mu_record_type_t *type, const mu_name_t *name) {
  for (size_t i = 0; i < type->argc; i++) {
    const mu_type_member_t member = type->argv[i];
    if (member.name != name)
      continue;
    return member.type;
  }

  return NULL;
}

static const mu_type_t *subsume(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b) {
  // If a and b are the same type, then just return
  if (a == b)
    return a;

  // Otherwise, ensure that either a or b is a variable type, or a and b have
  // the same kind
  CONTINUE(NULL, NULL, a, b);

  // Traverse a and b at the same time and equate each reachable couple
  do {
    for (;;) {
      int out = type_subsume(a, b, induce);

      if (out == TO_CONTINUE) {
        const mu_type_t *next_a = continue_into.a;
        const mu_type_t *next_b = continue_into.b;

        if (next_a == next_b)
          continue;

        if (next_a->kind != MU_VARIABLE_TYPE && next_b->kind != MU_VARIABLE_TYPE && next_a->kind != next_b->kind)
          assert(0);

        a = type_continue(a, next_a);
        b = type_continue(b, next_b);

        continue;
      }

      if (out == PROBLEM)
        assert(0);

      if (out == RETURN)
        break;
    }
  } while ((a = type_return(a)) != NULL && (b = type_return(b)) != NULL);

  // Ensure that type_return(b) is also NULL
  const mu_type_t *result = b;
  b = type_return(b);
  assert(b == NULL);

  return result;

/* except: */
/*   while ((a = type_return(a)) != NULL); */
/*   while ((b = type_return(b)) != NULL); */
  /* return NULL; */
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

static const mu_type_t *append(
    induce_t *induce,
    const mu_type_t *restrict lower,
    const mu_type_t *restrict upper) {
  for (size_t i = 0; i < induce->sub_length; i++) {
    induce_sub_t sub = induce->sub_data[i];
    if (sub.lower == lower && sub.upper == upper)
      return lower;
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
    .lower = lower, .upper = upper };
  return lower;
}

// ---------------------------------- Expr -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *access_expr_induce(
    const mu_access_expr_t *expr, induce_t *induce) {
  mu_engine_t *engine = induce->engine;

  const mu_variable_type_t *result;
  if ((result = mu_open_type(engine)) == NULL)
    return NULL;

  const mu_record_type_t *record_type;
  const mu_type_member_t argv[] = {
    { .name = expr->name, .type = &result->as_type }
  };
  size_t argc = sizeof(argv) / sizeof(argv[0]);
  if ((record_type = mu_record_type(engine, argc, argv)) == NULL)
    return NULL;

  const mu_type_t *matter_type = induce_evince(induce, &expr->matter->as_node);
  if (subsume(induce, matter_type, &record_type->as_type) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *boolean_expr_induce(
    const mu_boolean_expr_t *expr, induce_t *induce) {
  const mu_boolean_type_t *result;
  if ((result = mu_boolean_type(induce->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_expr_induce(
    const mu_integer_expr_t *expr, induce_t *induce) {
  const mu_integer_type_t *result;
  if ((result = mu_integer_type(induce->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *invoke_expr_induce(
    const mu_invoke_expr_t *expr, induce_t *induce) {
  const mu_type_t *lambda = induce_evince(induce, &expr->lambda->as_node);
  const mu_type_t *matter = induce_evince(induce, &expr->matter->as_node);

  const mu_variable_type_t *output;
  if ((output = mu_open_type(induce->engine)) == NULL)
    return NULL;
  const mu_type_t *result = &output->as_type;

  const mu_lambda_type_t *lambda_type;
  if ((lambda_type = mu_lambda_type(induce->engine, matter, result)) == NULL)
    return NULL;

  if (subsume(induce, lambda, &lambda_type->as_type) == NULL)
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

__attribute__((nonnull)) static const mu_type_t *name_expr_induce(
    const mu_name_expr_t *expr, induce_t *induce) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &expr->as_node)) != NULL)
    return induce_evince(induce, target);

  const mu_variable_type_t *result;
  if ((result = mu_open_type(induce->engine)) == NULL)
    return NULL;
  return &result->as_type;
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

__attribute__((nonnull)) static const mu_type_t *vector_expr_induce(
    const mu_vector_expr_t *expr, induce_t *induce) {
  const mu_variable_type_t *matter_type;
  if ((matter_type = mu_open_type(induce->engine)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = induce_evince(induce, &expr->argv[i]->as_node);
    if (subsume(induce, type, &matter_type->as_type) == NULL)
      return NULL;
  }

  const mu_vector_type_t *result;
  if ((result = mu_vector_type(induce->engine, &matter_type->as_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *zero_expr_induce(
    const mu_zero_expr_t *expr, induce_t *induce) {
  const mu_variable_type_t *result;
  if ((result = mu_open_type(induce->engine)) == NULL)
    return NULL;
  return &result->as_type;
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
  if ((result = mu_open_type(induce->engine)) == NULL)
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
  const mu_variable_type_t *open_type;
  if ((open_type = mu_open_type(induce->engine)) == NULL)
    return NULL;
  return &open_type->as_type;
}

// ---------------------------------- Type -------------------------------- {{{1

static int boolean_type_subsume(
    const mu_boolean_type_t *restrict a,
    const mu_type_t *restrict b,
    induce_t *induce) {
  return 1;
}

static int integer_type_subsume(
    const mu_integer_type_t *restrict a,
    const mu_type_t *restrict b,
    induce_t *induce) {
  return RETURN;
}

static int lambda_type_subsume(
    const mu_lambda_type_t *restrict a,
    const mu_type_t *restrict b,
    induce_t *induce) {
  assert(b->kind == MU_LAMBDA_TYPE);
  const mu_lambda_type_t *restrict rb = (const mu_lambda_type_t *) b;

  size_t i = type_cursor(&a->as_type)->i++, j = type_cursor(&rb->as_type)->i++;
  assert(i == j);

  if (i == 0)
    return CONTINUE(&a->as_type, b, rb->argument, a->argument);
  if (i == 1)
    return CONTINUE(&a->as_type, b, a->output, rb->output);

  return RETURN;
}

static int record_type_subsume(
    const mu_record_type_t *restrict a,
    const mu_type_t *restrict b,
    induce_t *induce) {
  assert(b->kind == MU_RECORD_TYPE);
  const mu_record_type_t *restrict rb = (const mu_record_type_t *) b;

  size_t j = type_cursor(&rb->as_type)->i++;

  if (j >= rb->argc)
    return RETURN;

  for (size_t i = 0; i < a->argc; i++) {
    if (a->argv[i].name == rb->argv[j].name) {
      type_cursor(&a->as_type)->i = i;
      return CONTINUE(&a->as_type, b, a->argv[i].type, rb->argv[j].type);
    }
  }

  return PROBLEM;
}

static int variable_type_subsume(
    const mu_variable_type_t *restrict a,
    const mu_type_t *restrict b,
    induce_t *induce) {
  size_t i;

  while ((i = type_cursor(&a->as_type)->i++) < induce->sub_length) {
    induce_sub_t sub = induce->sub_data[i];
    if (sub.upper != &a->as_type)
      continue;
    return CONTINUE(&a->as_type, b, sub.lower, b);
  }

  append(induce, &a->as_type, b);
  return RETURN;
}

static int variable_type_supersume(
    const mu_type_t *restrict a,
    const mu_variable_type_t *restrict b,
    induce_t *induce) {
  size_t j;

  while ((j = type_cursor(&b->as_type)->i++) < induce->sub_length) {
    induce_sub_t sub = induce->sub_data[j];
    if (sub.lower != &b->as_type)
      continue;
    return CONTINUE(a, &b->as_type, a, sub.upper);
  }

  append(induce, a, &b->as_type);
  return RETURN;
}

static int vector_type_subsume(
    const mu_vector_type_t *restrict a,
    const mu_type_t *restrict b,
    induce_t *induce) {
  assert(b->kind == MU_VECTOR_TYPE);
  const mu_vector_type_t *restrict rb = (const mu_vector_type_t *) b;

  size_t i = type_cursor(&a->as_type)->i++;
  if (i == 1)
    return RETURN;
  return CONTINUE(&a->as_type, b, a->matter, rb->matter);
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

static int type_subsume(
    const mu_type_t *restrict a,
    const mu_type_t *restrict b,
    induce_t *induce) {
  if (a->kind == MU_VARIABLE_TYPE)
    return variable_type_subsume((const mu_variable_type_t *) a, b, induce);

  if (b->kind == MU_VARIABLE_TYPE)
    return variable_type_supersume(a, (const mu_variable_type_t *) b, induce);

  if (a->kind != b->kind)
    return PROBLEM;

  switch (a->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_TYPE: \
      return lower##_type_subsume((const mu_##lower##_type_t *) a, b, induce);
    MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

// vim: set foldmethod=marker:
