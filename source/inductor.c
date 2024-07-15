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

/// Return the index where the next equivalent type to @a type should be
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
  const mu_type_t *root = type;

  size_t height = 0;
  for (const mu_type_t *next;; root = next) {
    if ((next = inductor_get(inductor, root)) == NULL)
      break;
    height++;
  }

  for (const mu_type_t *next; height-- > 0; type = next) {
    next = inductor->induce[slot(inductor, type)];
    inductor->induce[slot(inductor, type)] = root;
  }

  return root;
}

const mu_type_t *inductor_equate(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b) {
  // If a and b are the same type, then just return
  if (a == b)
    return a;

  a = inductor_root(inductor, a);
  b = inductor_root(inductor, b);

  // If a and b are both equivalent to the same type, then just return
  if (a == b)
    return a;

  // If a is an open type, then just equate it to b and return
  const mu_variable_type_t *va;
  if ((va = mu_type_cast(a, va)) != NULL && va->argc == 0)
    return inductor_set(inductor, a, b);

  // If b is an open type, then just equate it to a and return
  const mu_variable_type_t *vb;
  if ((vb = mu_type_cast(b, vb)) != NULL && vb->argc == 0)
    return inductor_set(inductor, b, a);

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

      next_a = inductor_root(inductor, next_a);
      next_b = inductor_root(inductor, next_b);

      // If next_a and next_b are both equivalent to the same type, then skip
      // them
      if (next_a == next_b)
        continue;

      // If next_a is an open type, then equate it to next_b and skip them
      if ((va = mu_type_cast(next_a, va)) != NULL && va->argc == 0) {
        if (inductor_set(inductor, next_a, next_b) == NULL)
          goto except;
        continue;
      }

      // If next_b is an open type, then equate it to next_a and skip them
      if ((vb = mu_type_cast(next_b, vb)) != NULL && vb->argc == 0) {
        if (inductor_set(inductor, next_a, next_b) == NULL)
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
    // equate them. We know we can do this with inductor_set because we only
    // continue into a root type.
    if (inductor_set(inductor, a, b) == NULL)
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

const mu_type_t *reduce_type(inductor_t *inductor, const mu_type_t *type) {
  assert(type_cursor(type)->anterior == NULL && type_cursor(type)->i == 0);
  const mu_type_t **reduce = inductor->reduce;

  // Return if the type, or a type equivalent to it, is already reduced. We
  // traverse the union-find structure, like inductor_root, to find each
  // equivalent type. However, at each step, we link the next equivalent type to
  // the previous one using the cursor. When we link an intermediate type like
  // this, the anterior type's cursor will have i == 0. Normally, an anterior
  // type's cursor can't have i == 0 because we always do type_cursor(type)->i++
  // before we type_continue from it.
  for (const mu_type_t *result;;) {
    if ((result = reduce[slot(inductor, type)]) != NULL) {
      while ((type = type_return(type)) != NULL)
        reduce[slot(inductor, type)] = result;
      return result;
    }

    if ((result = inductor_get(inductor, type)) == NULL)
      break;
    type = type_continue(type, result);
  }

  for (const mu_type_t *next, *result;;) {
    while ((next = type_at(type, type_cursor(type)->i++)) != NULL) {
      if (reduce[slot(inductor, next)] != NULL)
        continue;

      type = type_continue(type, next);

      while ((next = inductor_get(inductor, type)) != NULL)
        type = type_continue(type, next);
    }

    if ((result = type_reduce(type, inductor)) == NULL)
      goto except_type_reduce;

    // Unwind each equivalent type and record its reduce result
    do {
      reduce[slot(inductor, type)] = result;

      if ((type = type_return(type)) == NULL)
        return result;
    } while (type_cursor(type)->i == 0);
  };

except_type_reduce:
  while ((type = type_return(type)) != NULL);
  return NULL;
}

const mu_type_t *reduce_node(inductor_t *inductor, const mu_node_t *node) {
  const mu_type_t *type = inductor_node(inductor, node);
  assert(type != NULL);
  return reduce_type(inductor, type);
}

const mu_type_t *reduce_type_result(inductor_t *inductor, const mu_type_t *type) {
  if (slot(inductor, type) >= inductor->length)
    return type;
  return inductor->reduce[slot(inductor, type)];
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
  if (slot(inductor, source) >= inductor->length) {
    size_t length = slot(inductor, source) + 1;

    size_t next_size = sizeof(const mu_type_t *[length]);

    const mu_type_t **induce = inductor->induce;
    if ((induce = realloc(induce, next_size)) == NULL)
      return NULL;
    for (size_t i = inductor->length; i < length; i++)
      induce[i] = NULL;

    inductor->length = length;
    inductor->induce = induce;
  }

  return inductor->induce[slot(inductor, source)] = target;
}
