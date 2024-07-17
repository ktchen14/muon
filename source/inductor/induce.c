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

/// Induce the type of the abstract @a node with the @a induce engine
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

__attribute__((nonnull, pure))
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

static const mu_type_t *equate_continue(
    induce_t *induce,
    const mu_type_t *restrict *restrict a,
    const mu_type_t *restrict next_a,
    const mu_type_t *restrict *restrict b,
    const mu_type_t *restrict next_b) {
  if (next_a->kind != MU_VARIABLE_TYPE && next_b->kind != MU_VARIABLE_TYPE && next_a->kind != next_b->kind)
    assert(0);

  *a = type_continue(*a, next_a);
  *b = type_continue(*b, next_b);
  return next_a;
}

// Possible returns:
//   STOP - out of memory
//   NOTEQUAL - return from both types: types can't be equated
//   EQUAL - return from both types: types were equated
//   CONTINUE - continue trying to equate new types

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

  // Otherwise, ensure that either a or b is a variable type, or a and b have
  // the same kind
  equate_continue(induce, &(const mu_type_t *) {0}, a, &(const mu_type_t *) {0}, b);

  // Traverse a and b at the same time and equate each reachable couple
  do {
    for (;;) {
      const mu_variable_type_t *va = mu_type_cast(a, va);
      const mu_variable_type_t *vb = mu_type_cast(b, vb);

      if (va == NULL && vb == NULL) {
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

        equate_continue(induce, &a, next_a, &b, next_b);

      } else if (va != NULL && vb == NULL) {
        const mu_test_t *test;
        if ((test = variable_type_test_at(va, type_cursor(a)->i++)) == NULL) {
          if (set(induce, a, b) == NULL)
            goto except;
          goto type_return;
        }
        assert(test->kind == MU_MEMBER_TEST);
        const mu_member_test_t *member_test = (const mu_member_test_t *) test;

        const mu_record_type_t *rb = mu_type_cast(b, rb);
        assert(rb != NULL);

        const mu_type_t *next;
        if ((next = record_type_get_name(rb, member_test->name)) == NULL)
          assert(0);
        equate_continue(induce, &a, member_test->type, &b, next);

      } else if (vb != NULL && va == NULL) {
        const mu_test_t *test;
        if ((test = variable_type_test_at(vb, type_cursor(b)->i++)) == NULL) {
          if (set(induce, b, a) == NULL)
            goto except;
          goto type_return;
        }
        assert(test->kind == MU_MEMBER_TEST);
        const mu_member_test_t *member_test = (const mu_member_test_t *) test;

        const mu_record_type_t *ra = mu_type_cast(a, ra);
        assert(ra != NULL);

        const mu_type_t *next;
        if ((next = record_type_get_name(ra, member_test->name)) == NULL)
          assert(0);
        equate_continue(induce, &a, next, &b, member_test->type);

      } else if (va != NULL && vb != NULL) {
        assert(!"Unimplemented");
      }
    }

    // We should only return when next_a and next_b are both NULL. Here, we know
    // that all types reachable from both a and b have been equated. Next,
    // equate them. We know we can do this with set because we only
    // continue into a root type.
    if (set(induce, a, b) == NULL)
      goto except;
  type_return:;
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
  const mu_stmt_t *stmt;
  switch (node->kind) {
    case MU_NAME_EXPR_NODE:
    case MU_NAME_SIGN_NODE:
      if ((stmt = node_to_stmt[node->as_stator.id]) == NULL)
        return NULL;
      return i == 0 ? &stmt->as_node : NULL;
    default: return node_at(node, i);
  }
}

const mu_type_t *induce_node(induce_t *induce, const mu_node_t *root) {
  assert(root->as_stator.id < induce->node_number);

  if (induce->data[root->as_stator.id] != NULL)
    return induce->data[root->as_stator.id];

  const mu_node_t *node = root, *next;
  do {
    while ((next = indirect_at(node, node_cursor(node)->i++, induce->node_to_stmt)) != NULL) {
      assert(node->as_stator.id < induce->node_number);

      if (induce->data[node->as_stator.id] != NULL)
        continue;
      node = node_continue(node, next);
    }

    // Induce the type of the node
    const mu_type_t *type;
    if ((type = node_induce(node, induce)) == NULL)
      return NULL;
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

// ---------------------------------- Expr -------------------------------- {{{1

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

__attribute__((nonnull)) static const mu_type_t *name_expr_induce(
    const mu_name_expr_t *expr, induce_t *induce) {
  const mu_stmt_t *target;
  if ((target = induce->node_to_stmt[expr->as_stator.id]) != NULL)
    return induce_evince(induce, &target->as_node);

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

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *old_type = induce_evince(induce, &expr->argv[i]->as_node);
    const mu_member_type_t *mt = mu_type_cast(old_type, mt);
    assert(mt != NULL);

    const mu_name_t *name = mt->name;
    const mu_type_t *type = mt->matter;
    allocation->argv[i] = (mu_type_member_t) { .name = name, .type = type };
  }
  return &record_type_activate(allocation)->as_type;
}

__attribute__((nonnull)) static const mu_type_t *vector_expr_induce(
    const mu_vector_expr_t *expr, induce_t *induce) {
  const mu_variable_type_t *matter_type;
  if ((matter_type = mu_open_type(induce->engine)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = induce_evince(induce, &expr->argv[i]->as_node);
    if (equate(induce, &matter_type->as_type, type) == NULL)
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

__attribute__((nonnull)) static const mu_type_t *member_sign_induce(
    const mu_member_sign_t *sign, induce_t *induce) {
  assert(0);
}

__attribute__((nonnull)) static const mu_type_t *name_sign_induce(
    const mu_name_sign_t *sign, induce_t *induce) {
  const mu_stmt_t *target;
  if ((target = induce->node_to_stmt[sign->as_stator.id]) != NULL)
    return induce_evince(induce, &target->as_node);

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
