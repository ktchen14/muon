#include "induce.h"

#include "../stator.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Equate type @a to type @a b in the @a induce engine
 *
 * On allocation failure, @c errno is set by the allocator. This function can't
 * fail otherwise. The behavior is undefined if:
 *
 * - @a induce, @a a, or @a b is @c NULL
 * - @a a or @a b isn't in the same zone as that of the @a induce engine
 *
 * @param induce the induce engine
 * @param a the type to equate to @a b
 * @param b the type to equate to @a a
 * @return a type equivalent to both @a a and @a b on success; otherwise @c NULL
 */
static const mu_type_t *equate(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b)
  __attribute__((nonnull));

/// Return the root type equivalent to @a type in the @a induce context
static const mu_type_t *get_root(induce_t *induce, const mu_type_t *type)
  __attribute__((nonnull, returns_nonnull));

/// Set the next equivalent type of @a source to @a target in the @a induce
/// context
static const mu_type_t *set(induce_t *induce,
    const mu_type_t *restrict source,
    const mu_type_t *restrict target)
  __attribute__((nonnull));

/// Return the index where the next equivalent type to @a type should be
__attribute__((nonnull, pure))
static inline size_t slot(const induce_t *induce, const mu_type_t *type) {
  return induce->node_number + type->as_stator.id;
}

/// Induce the abstract @a node with the @a induce engine
static const mu_type_t *node_induce(const mu_node_t *node, induce_t *induce)
  __attribute__((nonnull));

induce_t *induce_initialize(
    induce_t *induce,
    mu_engine_t *engine,
    const mu_stmt_t *const *node_to_stmt,
    mu_status_t *status) {
  size_t length = engine->node_number + engine->type_number;

  const mu_type_t **data;
  if ((data = malloc(sizeof(const mu_type_t *[length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < length; data[i++] = NULL);

  *induce = (induce_t) {
    .engine = engine,
    .node_number = engine->node_number,
    .length = length,
    .data = data,
    .node_to_stmt = node_to_stmt,
    .status = status,
  };
  return induce;
}

const mu_type_t *get_root(induce_t *induce, const mu_type_t *type) {
  const mu_type_t *root = type;

  size_t height = 0;
  for (const mu_type_t *next;; root = next) {
    if ((next = induce_get(induce, root)) == NULL)
      break;
    height++;
  }

  for (const mu_type_t *next; height-- > 0; type = next) {
    next = induce->data[slot(induce, type)];
    induce->data[slot(induce, type)] = root;
  }

  return root;
}

static const mu_type_t *equate(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b) {
  // If a and b are the same type, then just return
  if (a == b)
    return a;

  a = get_root(induce, a);
  b = get_root(induce, b);

  // If a and b are both equivalent to the same type, then just return
  if (a == b)
    return a;

  // If a is an open type, then just equate it to b and return
  const mu_variable_type_t *va;
  if ((va = mu_type_cast(a, va)) != NULL) {
    if (b->kind == MU_MEMBER_TYPE)
      assert(0);

    if (va->argc == 0)
      return set(induce, a, b);

    // Fall through
  }

  // If b is an open type, then just equate it to a and return
  const mu_variable_type_t *vb;
  if ((vb = mu_type_cast(b, vb)) != NULL) {
    if (a->kind == MU_MEMBER_TYPE)
      assert(0);

    if (vb->argc == 0)
      return set(induce, b, a);

    // Fall through
  }

  // Otherwise, ensure that either a or b is a variable type, or a and b have
  // the same kind
  if (va == NULL && vb == NULL && a->kind != b->kind)
    assert(0);

  // Traverse a and b at the same time and equate each reachable couple
  do {
    for (;;) {
      const mu_type_t *next_a = type_at(a, type_cursor(a)->i++);
      const mu_type_t *next_b = type_at(b, type_cursor(b)->i++);

      if (next_a == NULL && next_b == NULL)
        break;

      if (next_a == NULL && next_b != NULL)
        assert(0);

      if (next_a != NULL && next_b == NULL)
        assert(0);

      // If next_a and next_b are the same type, then skip them
      if (next_a == next_b)
        continue;

      next_a = get_root(induce, next_a);
      next_b = get_root(induce, next_b);

      // If next_a and next_b are both equivalent to the same type, then skip
      // them
      if (next_a == next_b)
        continue;

      // If next_a is an open type, then equate it to next_b and skip them
      if ((va = mu_type_cast(next_a, va)) != NULL && va->argc == 0) {
        if (set(induce, next_a, next_b) == NULL)
          goto except;
        continue;
      }

      // If next_b is an open type, then equate it to next_a and skip them
      if ((vb = mu_type_cast(next_b, vb)) != NULL && vb->argc == 0) {
        if (set(induce, next_a, next_b) == NULL)
          goto except;
        continue;
      }

      // Otherwise, ensure that either next_a or next_b is a variable type, or
      // next_a and next_b have the same kind
      if (va == NULL && vb == NULL && next_a->kind != next_b->kind)
        assert(0);

      a = type_continue(a, next_a);
      b = type_continue(b, next_b);
    }

    // We should only return when next_a and next_b are both NULL. Here, we know
    // that all types reachable from both a and b have been equated. Next,
    // equate them. We know we can do this with set because we only
    // continue into a root type.
    if (set(induce, a, b) == NULL)
      goto except;
  } while ((a = type_return(a)) != NULL && (b = type_return(b)) != NULL);

  // Ensure that type_return(b) is also NULL
  const mu_type_t *result = b;
  b = type_return(b);
  assert(b == NULL);

  return result;

except:
  while ((a = type_return(a)) != NULL);
  while ((b = type_return(b)) != NULL);
  return NULL;
}

static inline const mu_node_t *indirect_at(
    const mu_node_t *node,
    size_t i,
    const mu_stmt_t *const *node_to_stmt) {
  if (node->kind == MU_NAME_EXPR_NODE) {
    const mu_stmt_t *stmt;
    if ((stmt = node_to_stmt[node->as_stator.id]) == NULL)
      return NULL;
    return i == 0 ? &stmt->as_node : NULL;
  } else
    return node_at(node, i);
}

const mu_type_t *induce_node(induce_t *induce, const mu_node_t *root) {
  const mu_node_t *node = root, *next;
  do {
    while ((next = indirect_at(node, node_cursor(node)->i++, induce->node_to_stmt)) != NULL)
      node = node_continue(node, next);

    // Induce the type of the node
    const mu_type_t *type;
    if ((type = node_induce(node, induce)) == NULL)
      return NULL;

    assert(node->as_stator.id < induce->node_number);
    induce->data[node->as_stator.id] = type;
  } while ((node = node_return(node)) != NULL);

  return induce_evince(induce, root);
}

const mu_type_t *induce_get(const induce_t *induce, const mu_type_t *type) {
  if (slot(induce, type) >= induce->length)
    return NULL;
  return induce->data[slot(induce, type)];
}

static const mu_type_t *set(
    induce_t *induce,
    const mu_type_t *restrict source,
    const mu_type_t *restrict target) {
  if (slot(induce, source) >= induce->length) {
    size_t length = slot(induce, source) + 1;

    size_t next_size = sizeof(const mu_type_t *[length]);

    const mu_type_t **data = induce->data;
    if ((data = realloc(data, next_size)) == NULL)
      return NULL;
    for (size_t i = induce->length; i < length; i++)
      data[i] = NULL;

    induce->length = length;
    induce->data = data;
  }

  return induce->data[slot(induce, source)] = target;
}

__attribute__((nonnull)) static const mu_type_t *access_expr_induce(
    const mu_access_expr_t *expr, induce_t *induce) {
  mu_engine_t *engine = induce->engine;

  const mu_variable_type_t *open_type;
  if ((open_type = mu_open_type(engine)) == NULL)
    return NULL;

  const mu_member_test_t *member_test;
  if ((member_test = mu_member_test(engine, expr->name, &open_type->as_type)) == NULL)
    return NULL;

  const mu_variable_type_t *variable_type;
  if ((variable_type = mu_variable_type(engine, 1, (const mu_test_t *[]) { &member_test->as_test })) == NULL)
    return NULL;

  const mu_type_t *matter = induce_evince(induce, &expr->matter->as_node);

  if (equate(induce, &variable_type->as_type, matter) == NULL)
    return NULL;

  return &open_type->as_type;
}

__attribute__((nonnull)) static const mu_type_t *boolean_expr_induce(
    const mu_boolean_expr_t *expr, induce_t *induce) {
  const mu_boolean_type_t *boolean_type;
  if ((boolean_type = mu_boolean_type(induce->engine)) == NULL)
    return NULL;
  return &boolean_type->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_expr_induce(
    const mu_integer_expr_t *expr, induce_t *induce) {
  const mu_integer_type_t *integer_type;
  if ((integer_type = mu_integer_type(induce->engine)) == NULL)
    return NULL;
  return &integer_type->as_type;
}

__attribute__((nonnull)) static const mu_type_t *member_expr_induce(
    const mu_member_expr_t *expr, induce_t *induce) {
  const mu_type_t *matter = induce_evince(induce, &expr->matter->as_node);

  mu_engine_t *engine = induce->engine;
  const mu_member_type_t *member_type;
  if ((member_type = mu_member_type(engine, expr->name, matter)) == NULL)
    return NULL;
  return &member_type->as_type;
}

__attribute__((nonnull)) static const mu_type_t *name_expr_induce(
    const mu_name_expr_t *expr, induce_t *induce) {
  const mu_stmt_t *target;
  if ((target = induce->node_to_stmt[expr->as_stator.id]) != NULL)
    return induce_evince(induce, &target->as_node);

  const mu_variable_type_t *open_type;
  if ((open_type = mu_open_type(induce->engine)) == NULL)
    return NULL;
  return &open_type->as_type;
}

__attribute__((nonnull)) const mu_type_t *record_expr_induce(
    const mu_record_expr_t *expr, induce_t *induce) {
  mu_engine_t *engine = induce->engine;

  mu_record_type_t *allocation;
  if ((allocation = record_type_allocate(engine, expr->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++)
    allocation->argv[i] = induce_evince(induce, &expr->argv[i]->as_node);
  return &record_type_activate(allocation)->as_type;
}

__attribute__((nonnull)) const mu_type_t *vector_expr_induce(
    const mu_vector_expr_t *expr, induce_t *induce) {
  const mu_variable_type_t *matter_type;
  if ((matter_type = mu_open_type(induce->engine)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = induce_evince(induce, &expr->argv[i]->as_node);
    if (equate(induce, &matter_type->as_type, type) == NULL)
      return NULL;
  }

  mu_engine_t *engine = induce->engine;
  const mu_vector_type_t *vector_type;
  if ((vector_type = mu_vector_type(engine, &matter_type->as_type)) == NULL)
    return NULL;
  return &vector_type->as_type;
}

__attribute__((nonnull)) static const mu_type_t *zero_expr_induce(
    const mu_zero_expr_t *expr, induce_t *induce) {
  const mu_variable_type_t *open_type;
  if ((open_type = mu_open_type(induce->engine)) == NULL)
    return NULL;
  return &open_type->as_type;
}

__attribute__((nonnull, pure)) static const mu_type_t *define_stmt_induce(
    const mu_define_stmt_t *stmt, induce_t *induce) {
  return induce_evince(induce, &stmt->expr->as_node);
}

__attribute__((nonnull)) static const mu_type_t *type_stmt_induce(
    const mu_type_stmt_t *stmt, induce_t *induce) {
  assert(0);
}

static const mu_type_t *node_induce(const mu_node_t *node, induce_t *induce) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_EXPR_NODE: \
      return lower##_expr_induce((const mu_##lower##_expr_t *) node, induce);
    MU_EACH_EXPR_KIND(MU_EMIT)
#undef MU_EMIT

#define MU_EMIT(lower, upper, t) case MU_##upper##_SIGN: return NULL;
    MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT

#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_STMT_NODE: \
      return lower##_stmt_induce((const mu_##lower##_stmt_t *) node, induce);
    MU_EACH_STMT_KIND(MU_EMIT)
#undef MU_EMIT
  }

  __builtin_unreachable();
}
