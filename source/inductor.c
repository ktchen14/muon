#include "inductor.h"

#include "stator.h"
#include "status.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/// Get the next type equivalent to @a type in the @a inductor
static const mu_type_t *inductor_get(
    const inductor_t *inductor, const mu_type_t *type)
  __attribute__((nonnull, pure));

/// Set the next equivalent type of @a source to @a target in the @a inductor
static const mu_type_t *inductor_set(
    inductor_t *inductor,
    const mu_type_t *restrict source,
    const mu_type_t *restrict target)
  __attribute__((nonnull));

__attribute__((nonnull, pure))
static inline size_t slot(const inductor_t *inductor, const mu_type_t *type) {
  return inductor->node_number + type->as_stator.id;
}

inductor_t *inductor_initialize(
    inductor_t *inductor,
    mu_engine_t *engine,
    const mu_stmt_t *const *node_to_stmt,
    mu_status_t *status) {
  size_t length = engine->node_number + engine->type_number;

  const mu_type_t **induce;
  if ((induce = malloc(sizeof(const mu_type_t *[length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < length; induce[i++] = NULL);

  *inductor = (inductor_t) {
    .engine = engine,
    .node_number = engine->node_number,
    .length = length,
    .induce = induce,
    .node_to_stmt = node_to_stmt,
    .status = status,
  };
  return inductor;
}

void inductor_raze(inductor_t *inductor) {
  free(inductor->induce);
}

const mu_type_t *inductor_root(inductor_t *inductor, const mu_type_t *type) {
  const mu_type_t *origin = type;

  size_t height = 0;
  for (const mu_type_t *next;; type = next) {
    if ((next = inductor_get(inductor, type)) == NULL)
      break;
    height++;
  }

  const mu_type_t *root = type;

  type = origin;
  for (size_t i = 0; i < height; i++) {
    const mu_type_t *next = inductor->induce[slot(inductor, type)];
    inductor->induce[slot(inductor, type)] = root;
    type = next;
  }

  return root;
}


const mu_type_t *inductor_equate(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b) {
  if (a == b)
    return a;

  a = inductor_root(inductor, a);
  b = inductor_root(inductor, b);

  // If a and b refer to the same node/type
  if (a == b)
    return a;

  // If a is a variable type, then just equate it to b
  if (a->kind == MU_VARIABLE_TYPE)
    return inductor_set(inductor, a, b);

  if (b->kind == MU_VARIABLE_TYPE)
    return inductor_set(inductor, b, a);

  // Otherwise, both a and b are concrete types. If they don't have the same
  // kind, then they can't be unified.
  if (a->kind != b->kind)
    assert(0);

  do {
    for (;;) {
      const mu_type_t *next_a = type_at(a, type_cursor(a)->i++);
      const mu_type_t *next_b = type_at(b, type_cursor(b)->i++);

      if (next_a == NULL && next_b == NULL) {
        break;
      } else if (next_a == NULL && next_b != NULL) {
        assert(0);
      } else if (next_a != NULL && next_b == NULL) {
        assert(0);
      } else if (next_a != NULL && next_b != NULL) {
        if (next_a == next_b)
          break;

        if (next_a->kind == MU_VARIABLE_TYPE) {
          inductor_set(inductor, next_a, next_b);
          break;
        }

        if (next_b->kind == MU_VARIABLE_TYPE) {
          inductor_set(inductor, next_a, next_b);
          break;
        }

        if (next_a->kind != next_b->kind) {
          assert(0);
        }

        a = type_continue(a, next_a);
        b = type_continue(b, next_b);
      }
    }

    // Swap a and b if b is a variable type
    if (b->kind == MU_VARIABLE_TYPE) {
      const mu_type_t *t = a;
      a = b;
      b = t;
    }

    // Now that we've unified each type within a and b, unify them
    inductor_set(inductor, a, b);
  } while ((a = type_return(a)) != NULL && (b = type_return(b)) != NULL);

  return b;
}

static const mu_type_t *inductor_get(
    const inductor_t *inductor, const mu_type_t *type) {
  if (slot(inductor, type) >= inductor->length)
    return NULL;
  return inductor->induce[slot(inductor, type)];
}

static const mu_type_t *inductor_set(
    inductor_t *inductor,
    const mu_type_t *restrict source,
    const mu_type_t *restrict target) {
  size_t origin = inductor->length;

  if (slot(inductor, source) < origin)
    return inductor->induce[slot(inductor, source)] = target;

  size_t length = slot(inductor, source) + 1;

  const mu_type_t **induce = inductor->induce;
  if ((induce = realloc(induce, sizeof(const mu_type_t *[length]))) == NULL)
    return NULL;
  for (size_t i = origin; i < length; induce[i++] = NULL);

  inductor->length = length;
  inductor->induce = induce;
  return inductor->induce[slot(inductor, source)] = target;
}

const mu_type_t *inductor_type_of_node(
    inductor_t *inductor, const mu_node_t *node) {
  const mu_type_t *root = inductor_node(inductor, node);
  assert(root != NULL);

  root = inductor_root(inductor, root);
  assert(root != NULL);

  const mu_type_t *type = root, *next;
  do {
    while ((next = type_at(type, type_cursor(type)->i++)) != NULL) {
      const mu_type_t *result = inductor_root(inductor, next);
      type = type_continue(type, result);
    }

    const mu_type_t *result;
    if ((result = type_reduce(type, inductor)) == NULL)
      return NULL;

    if (result == type)
      continue;
    inductor_set(inductor, type, result);
  } while ((type = type_return(type)) != NULL);

  return inductor_root(inductor, root);
}

const mu_type_t *induce_node(inductor_t *inductor, const mu_node_t *root) {
  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    // Induce the type of the node
    const mu_type_t *type;
    if ((type = node_induce(node, inductor)) == NULL)
      return NULL;

    // If the node already had a type assigned to it, then we have to equate
    // this induced type with the existing one
    const mu_type_t *extant;
    if ((extant = inductor_node(inductor, node)) == NULL)
      inductor_node(inductor, node) = type;
    else if (inductor_equate(inductor, extant, type) == NULL)
      return NULL;
  } while ((node = node_return(node)) != NULL);

  return inductor_node(inductor, root);
}
