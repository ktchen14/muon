#include "common.h"
#include "engine.h"
#include "inductor.h"
#include "stator.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct inductor_member_t {
  /// Whether the target stator is a node or type
  enum {
    INDUCTOR_NONE, INDUCTOR_NODE, INDUCTOR_TYPE,
  } kind;

  size_t id;
  size_t index;

  union {
    const mu_stator_t *stator;
    const mu_node_t *node;
    const mu_type_t *type;
  };
} member_t;

static const member_t member_none = {0};

__attribute__((nonnull, pure))
static inline size_t inductor_length(const inductor_t *inductor) {
  return inductor->node_length + inductor->type_length;
}

/// Create a node member
__attribute__((nonnull))
static inline member_t node_member(
    const inductor_t *inductor, const mu_node_t *node) {
  assert(node->as_stator.id < inductor->node_length);
  size_t index = node->as_stator.id;
  return (member_t) { INDUCTOR_NODE, .id = node->as_stator.id, .index = index, .node = node };
}

/// Create a type member
__attribute__((nonnull))
static inline member_t type_member(
    const inductor_t *inductor, const mu_type_t *type) {
  size_t index = inductor->node_length + type->as_stator.id;
  return (member_t) { INDUCTOR_TYPE, .id = type->as_stator.id, .index = index, .type = type };
}

static const member_t *inductor_get(
    const inductor_t *inductor, const member_t *member)
  __attribute__((nonnull, pure, returns_nonnull));

static const member_t *inductor_set(
    inductor_t *inductor,
    const member_t *restrict source,
    const member_t *restrict target)
  __attribute__((nonnull));

static inductor_t *inductor_equate(
    inductor_t *inductor, const member_t *a, const member_t *b)
  __attribute__((nonnull));

static const member_t *inductor_root(
    inductor_t *inductor, const member_t *member)
  __attribute__((nonnull));

inductor_t *inductor_initialize(inductor_t *inductor, mu_engine_t *engine) {
  size_t length = engine->node_number + engine->type_number + 200;

  member_t *data;
  if ((data = malloc(sizeof(member_t[length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < length; data[i++] = (member_t) {0});

  *inductor = (inductor_t) {
    .engine = engine,
    .node_length = engine->node_number,
    .type_length = engine->type_number + 200,
    .data = data,
  };
  return inductor;
}

inductor_t *inductor_equate_node_node(
    inductor_t *inductor, const mu_node_t *a, const mu_node_t *b) {
  member_t i = node_member(inductor, a);
  member_t j = node_member(inductor, b);
  return inductor_equate(inductor, &i, &j);
}

inductor_t *inductor_equate_node_type(
    inductor_t *inductor, const mu_node_t *a, const mu_type_t *b) {
  member_t i = node_member(inductor, a);
  member_t j = type_member(inductor, b);
  return inductor_equate(inductor, &i, &j);
}

inductor_t *inductor_equate_type_type(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b) {
  member_t i = type_member(inductor, a);
  member_t j = type_member(inductor, b);
  return inductor_equate(inductor, &i, &j);
}

const member_t *inductor_get(
    const inductor_t *inductor, const member_t *member) {
  if (member->index >= inductor_length(inductor))
    return &member_none;
  return &inductor->data[member->index];
}

const member_t *inductor_set(
    inductor_t *inductor,
    const member_t *restrict source,
    const member_t *restrict target) {
  size_t origin = inductor_length(inductor);

  if (source->index < origin) {
    inductor->data[source->index] = *target;
    return target;
  }

  fprintf(stderr, "Reallocating\n");
  size_t length = inductor->node_length + source->id + 200;

  member_t *data = inductor->data;
  if ((data = realloc(data, sizeof(member_t[length]))) == NULL)
    return NULL;
  for (size_t i = origin; i < length; data[i++] = (member_t) {0});

  inductor->type_length = source->id + 200;
  inductor->data = data;

  inductor->data[inductor->node_length + source->id] = *target;
  return target;
}

static const member_t *inductor_root(
    inductor_t *inductor, const member_t *member) {
  for (;;) {
    const member_t *next = inductor_get(inductor, member);
    if (next->kind == INDUCTOR_NONE)
      return member;
    member = next;
  }

  return member;
}

const mu_type_t *inductor_type_of_node(inductor_t *inductor, const mu_node_t *node) {
  member_t member = node_member(inductor, node);
  const member_t *root_member = inductor_root(inductor, &member);

  // TODO: create a type variable
  if (root_member->kind == INDUCTOR_NODE)
    return NULL;

  const mu_type_t *type = root_member->type;

  do {
    const mu_type_t *next;
    while ((next = type_at(type, type_cursor(type)->i++)) != NULL) {
      const member_t *result;
      member = type_member(inductor, next);
      result = inductor_root(inductor, &member);
      assert(result->kind == INDUCTOR_TYPE);
      type = type_continue(type, result->type);
    }

    const mu_type_t *result = type_reduce(type, inductor);
    if (result != type) {
      member_t i = type_member(inductor, type);
      member_t j = type_member(inductor, result);
      inductor_set(inductor, &i, &j);
    }
  } while ((type = type_return(type)) != NULL);

  member = node_member(inductor, node);
  root_member = inductor_root(inductor, &member);
  assert(root_member->kind == INDUCTOR_TYPE);
  return root_member->type;
}

static inductor_t *inductor_equate(
    inductor_t *inductor, const member_t *a, const member_t *b) {
  a = inductor_root(inductor, a);
  b = inductor_root(inductor, b);

  if (a == b)
    return inductor;

  if (a->kind == INDUCTOR_NODE && b->kind == INDUCTOR_NODE) {
    if (a->node == b->node)
      return inductor;
    return inductor_set(inductor, a, b), inductor;
  }

  if (a->kind == INDUCTOR_NODE && b->kind == INDUCTOR_TYPE)
    return inductor_set(inductor, a, b), inductor;

  if (a->kind == INDUCTOR_TYPE && b->kind == INDUCTOR_NODE)
    return inductor_set(inductor, b, a), inductor;

  // If a and b are already unified then we're done
  if (a->type == b->type)
    return inductor;

  // If a is a variable type, then just equate it to b
  if (a->type->kind == MU_VARIABLE_TYPE)
    return inductor_set(inductor, a, b), inductor;

  if (b->type->kind == MU_VARIABLE_TYPE)
    return inductor_set(inductor, b, a), inductor;

  // Otherwise, both a and b are concrete types. If they don't have the same
  // kind, then they can't be unified.
  if (a->type->kind != b->type->kind)
    assert(0);

  const mu_type_t *type_a = a->type, *type_b = b->type;
  do {
    for (;;) {
      const mu_type_t *next_a = type_at(type_a, type_cursor(type_a)->i++);
      const mu_type_t *next_b = type_at(type_b, type_cursor(type_b)->i++);

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
          member_t i = type_member(inductor, next_a);
          member_t j = type_member(inductor, next_b);
          inductor_set(inductor, &i, &j);
          break;
        }

        if (next_b->kind == MU_VARIABLE_TYPE) {
          member_t i = type_member(inductor, next_a);
          member_t j = type_member(inductor, next_b);
          inductor_set(inductor, &j, &i);
          break;
        }

        if (next_a->kind != next_b->kind)
          assert(0);

        type_a = type_continue(type_a, next_a);
        type_b = type_continue(type_b, next_b);
      }
    }

    // Swap type_a and type_b if type_b is a variable type
    if (type_b->kind == MU_VARIABLE_TYPE) {
      const mu_type_t *t = type_a;
      type_a = type_b;
      type_b = t;
    }

    // Now that we've unified each type within type_a and type_b, unify them
    member_t i = type_member(inductor, type_a);
    member_t j = type_member(inductor, type_b);
    inductor_set(inductor, &i, &j);
  } while ((type_a = type_return(type_a)) != NULL && (type_b = type_return(type_b)) != NULL);

  return inductor;
}

const mu_type_t *inductor_type_root(
    inductor_t *inductor, const mu_type_t *type) {
  member_t member = type_member(inductor, type);

  const member_t *result;
  result = inductor_root(inductor, &member);
  assert(result->kind == INDUCTOR_TYPE);

  return result->type;
}
