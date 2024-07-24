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

static const mu_type_t *next_lower(
    const induce_t *induce, size_t *index, const mu_type_t *upper)
  __attribute__((nonnull, pure));

static const mu_type_t *next_upper(
    const induce_t *induce, size_t *index, const mu_type_t *lower)
  __attribute__((nonnull, pure));

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

#define PROBLEM 0
#define RETURN 1
#define TO_CONTINUE 2

static int continue_into(induce_t *induce, const mu_type_t *a, const mu_type_t *b) {
  induce->next_a = a;
  induce->next_b = b;
  return TO_CONTINUE;
}

/// Induce the type of the abstract @a node with the @a induce engine
static const mu_type_t *node_induce(const mu_node_t *node, induce_t *induce)
  __attribute__((nonnull));

/**
 * @brief Nominate the abstract type @a a as a subtype of the abstract type @a b
 *
 * This will dispatch to a concrete implementation for each type kind of @a a.
 * Then, it will return a positive number if @a a is (always or already) a
 * subtype of @a b, zero if @a a isn't (always or already) a subtype of @a b, or
 * a negative number if we must continue into @a a and @a b to answer.
 *
 * @return an indication of whether type @a a is a subtype of type @a b
 */
__attribute__((nonnull))
static int nominate(
    const induce_t *induce, const mu_type_t *a, const mu_type_t *b) {
  if (a == b)
    return 1;

  if (a->kind != MU_VARIABLE_TYPE && b->kind != MU_VARIABLE_TYPE && a->kind != b->kind) {
    fprintf(stderr, "Type ");
    mu_type_debug(a);
    fprintf(stderr, " isn't a subtype of ");
    mu_type_debug(b);
    fprintf(stderr, "\n");

    return 0;
  }

  for (size_t i = 0; i < induce->sub_length; i++) {
    if (induce->sub_data[i].lower == a && induce->sub_data[i].upper == b)
      return 1;
  }
  return -1;
}

static int type_restrict(
    const mu_type_t *restrict a, const mu_type_t *restrict b,
    induce_t *induce)
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
  assert(type_cursor(a)->anterior == NULL && type_cursor(a)->i == 0);
  assert(type_cursor(b)->anterior == NULL && type_cursor(b)->i == 0);

  // If we don't have to continue into a or b, then just return
  if (nominate(induce, a, b) >= 0)
    return induce;

  for (const mu_type_t *last_a = NULL, *last_b = NULL;;) {
    int e;

    if (a->kind != MU_VARIABLE_TYPE && b->kind != MU_VARIABLE_TYPE) {
      const mu_type_t *next_a, *next_b;
      do {
        if ((e = type_restrict(a, b, induce)) == RETURN)
          goto next;
        if (e == PROBLEM)
          goto except;
        assert(e == TO_CONTINUE);

        next_a = induce->next_a;
        next_b = induce->next_b;
      } while ((e = nominate(induce, next_a, next_b)) >= 0);

      if (e == 0)
        goto rollback;

      a = type_continue(a, next_a);
      b = type_continue(b, next_b);

    } else if (a->kind == MU_VARIABLE_TYPE) {
      // Before we restrict a variable type α to be a subtype of β, we must also
      // restrict *each subtype of α* to be a subtype of β. That is:
      //
      //   α <: β   if and only if   ∀(τ | τ <: α) τ <: β
      //
      // Select the next τ that's an immediate subtype of α and isn't, already
      // or trivially, a subtype of β. Then, resume the main loop with:
      //
      //   (a, b) = (τ, β)
      //
      // If some τ can't be made a subtype of β, just roll back as α <: β isn't
      // true. Once we've exhausted all subtypes of α, register α <: β.
      //
      // Note that when we resume the main loop with (τ, β) from (α, β), we
      // can't "push" another instance of β. That is, we can't do:
      //
      //   β = type_continue(β, β)
      //
      // Because the cursor in the type header of β is already in use by this
      // iteration of the main loop. As a result, when we return from (τ, β),
      // we'll return to (α, β') where β' is the type that preceded β. In this
      // case, we need to "repush" β before we move on.

      if (last_b != NULL)
        b = type_continue(b, last_b);

      const mu_type_t *next_a;
      do {
        if ((next_a = next_lower(induce, &type_cursor(a)->i, a)) == NULL)
          goto append;
      } while ((e = nominate(induce, next_a, b)) >= 0);

      if (e == 0)
        goto rollback;

      a = type_continue(a, next_a);

    } else if (b->kind == MU_VARIABLE_TYPE) {
      // Before we restrict α to be a subtype of a variable type β, we must also
      // restrict each α to be a subtype of *each supertype of β*. That is:
      //
      //   α <: β   if and only if   ∀(τ | β <: τ) a <: τ
      //
      // Select the next τ that's an immediate supertype of β and isn't, already
      // or trivially, a supertype of α. Then, resume the main loop with:
      //
      //   (a, b) = (α, τ)
      //
      // If α can't be made a subtype of some τ, just roll back as α <: β isn't
      // true. Once we've exhausted all supertypes of β, register α <: β.
      //
      // Note that when we resume the main loop with (α, τ) from (α, β), we
      // can't "push" another instance of α. That is, we can't do:
      //
      //   α = type_continue(α, α)
      //
      // Because the cursor in the type header of α is already in use by this
      // iteration of the main loop. As a result, when we return from (α, τ),
      // we'll return to (α', β) where α' is the type that preceded α. In this
      // case, we need to "repush" α before we move on.

      if (last_a != NULL)
        a = type_continue(a, last_a);

      const mu_type_t *next_b;
      do {
        if ((next_b = next_upper(induce, &type_cursor(b)->i, b)) == NULL)
          goto append;
      } while ((e = nominate(induce, a, next_b)) >= 0);

      if (e == 0)
        goto rollback;

      b = type_continue(b, next_b);
    }

    last_a = last_b = NULL;
    continue;

  append:
    if (append(induce, a, b) == NULL)
      goto except;

  next:
    a = type_return(last_a = a);
    b = type_return(last_b = b);
    if (a == NULL && b == NULL)
      break;
  }

  return induce;

rollback:
  while ((a = type_return(a)) != NULL);
  while ((b = type_return(b)) != NULL);
  return induce;

except:
  while ((a = type_return(a)) != NULL);
  while ((b = type_return(b)) != NULL);
  return NULL;
}

static const mu_type_t *next_lower(
    const induce_t *induce, size_t *index, const mu_type_t *upper) {
  size_t i;
  while ((i = (*index)++) < induce->sub_length) {
    if (induce->sub_data[i].upper == upper)
      return induce->sub_data[i].lower;
  }
  return NULL;
}

static const mu_type_t *next_upper(
    const induce_t *induce, size_t *index, const mu_type_t *lower) {
  size_t i;
  while ((i = (*index)++) < induce->sub_length) {
    if (induce->sub_data[i].lower == lower)
      return induce->sub_data[i].upper;
  }
  return NULL;
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

__attribute__((nonnull)) static const mu_type_t *access_expr_induce(
    const mu_access_expr_t *expr, induce_t *induce) {
  mu_engine_t *engine = induce->engine;

  const mu_variable_type_t *result;
  if ((result = mu_variable_type(engine)) == NULL)
    return NULL;

  const mu_record_type_t *record_type;
  const mu_type_member_t argv[] = {
    { .name = expr->name, .type = &result->as_type }
  };
  size_t argc = sizeof(argv) / sizeof(argv[0]);
  if ((record_type = mu_record_type(engine, argc, argv)) == NULL)
    return NULL;

  const mu_type_t *matter_type = induce_evince(induce, &expr->matter->as_node);
  if (restrict_type(induce, matter_type, &record_type->as_type) == NULL)
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
  if ((output = mu_variable_type(induce->engine)) == NULL)
    return NULL;
  const mu_type_t *result = &output->as_type;

  const mu_lambda_type_t *lambda_type;
  if ((lambda_type = mu_lambda_type(induce->engine, matter, result)) == NULL)
    return NULL;

  if (restrict_type(induce, lambda, &lambda_type->as_type) == NULL)
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
  if ((result = mu_variable_type(induce->engine)) == NULL)
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
  if ((matter_type = mu_variable_type(induce->engine)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = induce_evince(induce, &expr->argv[i]->as_node);
    if (restrict_type(induce, type, &matter_type->as_type) == NULL)
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
  if ((result = mu_variable_type(induce->engine)) == NULL)
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

// ---------------------------------- Type -------------------------------- {{{1

__attribute__((nonnull)) static int boolean_type_restrict(
    const mu_boolean_type_t *restrict a, const mu_type_t *restrict b,
    induce_t *induce) {
  return RETURN;
}

__attribute__((nonnull)) static int integer_type_restrict(
    const mu_integer_type_t *restrict a, const mu_type_t *restrict b,
    induce_t *induce) {
  return RETURN;
}

__attribute__((nonnull)) static int lambda_type_restrict(
    const mu_lambda_type_t *restrict a, const mu_type_t *restrict b,
    induce_t *induce) {
  assert(b->kind == MU_LAMBDA_TYPE);
  const mu_lambda_type_t *restrict rb = (const mu_lambda_type_t *) b;

  switch (type_cursor(&a->as_type)->i++) {
    case 0: return continue_into(induce, rb->argument, a->argument);
    case 1: return continue_into(induce, a->output, rb->output);
    default: return RETURN;
  }
}

__attribute__((nonnull)) static int record_type_restrict(
    const mu_record_type_t *restrict a, const mu_type_t *restrict b,
    induce_t *induce) {
  assert(b->kind == MU_RECORD_TYPE);
  const mu_record_type_t *restrict rb = (const mu_record_type_t *) b;

  size_t j;

  if ((j = type_cursor(&rb->as_type)->i++) >= rb->argc)
    return RETURN;

  for (size_t i = 0; i < a->argc; i++) {
    if (a->argv[i].name == rb->argv[j].name)
      return continue_into(induce, a->argv[i].type, rb->argv[j].type);
  }

  return PROBLEM;
}

__attribute__((nonnull)) static int variable_type_restrict(
    const mu_variable_type_t *restrict a, const mu_type_t *restrict b,
    induce_t *induce) {
  __builtin_unreachable();
}

__attribute__((nonnull)) static int vector_type_restrict(
    const mu_vector_type_t *restrict a, const mu_type_t *restrict b,
    induce_t *induce) {
  assert(b->kind == MU_VECTOR_TYPE);
  const mu_vector_type_t *restrict rb = (const mu_vector_type_t *) b;

  if (type_cursor(&a->as_type)->i++ == 1)
    return RETURN;
  return continue_into(induce, a->matter, rb->matter);
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

static int type_restrict(
    const mu_type_t *restrict a,
    const mu_type_t *restrict b,
    induce_t *induce) {
  switch (a->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_TYPE: \
      return lower##_type_restrict((const mu_##lower##_type_t *) a, b, induce);
    MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}

// vim: set foldmethod=marker:
