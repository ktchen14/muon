#include "common.h"
#include "inductor.h"
#include "stator.h"
#include "status.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct inductor_member_t {
  /// Whether the target stator is a node or type
  enum { NONE, NODE, TYPE, } kind;

  union {
    /// Node that the member represents (if kind is NODE)
    const mu_node_t *node;

    /// Type that the member represents (if kind is TYPE)
    const mu_type_t *type;
  };

  /// Index within the inductor's data member that should hold the next member
  /// equivalent to this member
  size_t index;
} member_t;

/// @internal Create an inductor member from the @a node
static member_t node_member(const inductor_t *inductor, const mu_node_t *node)
  __attribute__((nonnull));

/// @internal Create an inductor member from the @a type
static member_t type_member(const inductor_t *inductor, const mu_type_t *type)
  __attribute__((nonnull));

/// Get the next member
static member_t inductor_get(const inductor_t *inductor, member_t member)
  __attribute__((nonnull, pure));

/// Set the next member
static inductor_t *inductor_set(
    inductor_t *inductor, member_t source, member_t target)
  __attribute__((nonnull));

/// Equate @a a and @a b
static inductor_t *inductor_equate(inductor_t *inductor, member_t a, member_t b)
  __attribute__((nonnull));

/// Return the archetypal member
static member_t inductor_root(inductor_t *inductor, member_t member)
  __attribute__((nonnull));

inductor_t *inductor_initialize(
    inductor_t *inductor,
    mu_engine_t *engine,
    const mu_stmt_t *const *node_to_stmt,
    mu_status_t *status) {
  size_t length = engine->node_number + engine->type_number;
  member_t *data;
  if ((data = malloc(sizeof(member_t[length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < length; data[i++] = (member_t) {0});

  *inductor = (inductor_t) {
    .engine = engine,
    .node_number = engine->node_number,
    .length = length,
    .data = data,
    .node_to_stmt = node_to_stmt,
    .status = status,
  };
  return inductor;
}

void inductor_raze(inductor_t *inductor) {
  free(inductor->data);
}

const mu_type_t *inductor_type_root(
    inductor_t *inductor, const mu_type_t *type) {
  member_t result = inductor_root(inductor, type_member(inductor, type));
  assert(result.kind == TYPE);
  return result.type;
}

const mu_type_t *inductor_type_of_node(inductor_t *inductor, const mu_node_t *node) {
  member_t member = inductor_root(inductor, node_member(inductor, node));

  // If the root member of the node isn't a type, then we need to create a
  // variable type to become the type of the node
  if (member.kind == NODE) {
    const mu_variable_type_t *variable_type;
    if ((variable_type = mu_variable_type(inductor->engine)) == NULL)
      return NULL;

    member_t next = type_member(inductor, &variable_type->as_type);
    if (inductor_set(inductor, member, next) == NULL)
      return NULL;

    return next.type;
  }

  const mu_type_t *type = member.type, *next;
  do {
    while ((next = type_at(type, type_cursor(type)->i++)) != NULL) {
      member_t result = inductor_root(inductor, type_member(inductor, next));
      assert(result.kind == TYPE);
      type = type_continue(type, result.type);
    }

    const mu_type_t *result = type_reduce(type, inductor);
    if (result != type) {
      member_t i = type_member(inductor, type);
      member_t j = type_member(inductor, result);
      inductor_set(inductor, i, j);
    }
  } while ((type = type_return(type)) != NULL);

  member = inductor_root(inductor, node_member(inductor, node));
  assert(member.kind == TYPE);
  return member.type;
}

inductor_t *inductor_equate_node_node(
    inductor_t *inductor, const mu_node_t *a, const mu_node_t *b) {
  member_t i = node_member(inductor, a);
  member_t j = node_member(inductor, b);
  return inductor_equate(inductor, i, j);
}

inductor_t *inductor_equate_node_type(
    inductor_t *inductor, const mu_node_t *a, const mu_type_t *b) {
  member_t i = node_member(inductor, a);
  member_t j = type_member(inductor, b);
  return inductor_equate(inductor, i, j);
}

inductor_t *inductor_equate_type_type(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b) {
  member_t i = type_member(inductor, a);
  member_t j = type_member(inductor, b);
  return inductor_equate(inductor, i, j);
}

static member_t inductor_get(const inductor_t *inductor, member_t member) {
  if (member.index >= inductor->length)
    return (member_t) {0};
  return inductor->data[member.index];
}

static inductor_t *inductor_set(
    inductor_t *inductor, member_t source, member_t target) {
  size_t origin = inductor->length;

  if (source.index < origin) {
    inductor->data[source.index] = target;
    return inductor;
  }

  size_t length = source.index + 1;

  member_t *data = inductor->data;
  if ((data = realloc(data, sizeof(member_t[length]))) == NULL)
    return NULL;
  for (size_t i = origin; i < length; data[i++] = (member_t) {0});

  inductor->length = length;
  inductor->data = data;
  inductor->data[source.index] = target;
  return inductor;
}

static member_t inductor_root(inductor_t *inductor, member_t member) {
  member_t origin = member;

  size_t height = 0;
  member = origin;
  for (member_t next;; member = next) {
    if ((next = inductor_get(inductor, member)).kind == NONE)
      break;
    height++;
  }

  member_t root = member;

  member = origin;
  for (size_t i = 0; i < height; i++) {
    member_t next = inductor->data[member.index];
    inductor->data[member.index] = root;
    member = next;
  }

  return root;
}

static inductor_t *inductor_equate(inductor_t *inductor, member_t a, member_t b) {
  a = inductor_root(inductor, a);
  b = inductor_root(inductor, b);

  // If a and b refer to the same node/type
  if (a.index == b.index)
    return inductor;

  // If both a and b are nodes, then a = b unless they're the same node
  if (a.kind == NODE && b.kind == NODE)
    return inductor_set(inductor, a, b), inductor;

  // If a is a node and b is a type, then a = b
  if (a.kind == NODE && b.kind == TYPE)
    return inductor_set(inductor, a, b), inductor;

  // If a is a type and b is a node, then b = a
  if (a.kind == TYPE && b.kind == NODE)
    return inductor_set(inductor, b, a), inductor;

  // If a is a variable type, then just equate it to b
  if (a.type->kind == MU_VARIABLE_TYPE)
    return inductor_set(inductor, a, b), inductor;

  if (b.type->kind == MU_VARIABLE_TYPE)
    return inductor_set(inductor, b, a), inductor;

  // Otherwise, both a and b are concrete types. If they don't have the same
  // kind, then they can't be unified.
  if (a.type->kind != b.type->kind)
    assert(0);

  const mu_type_t *type_a = a.type, *type_b = b.type;
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
          inductor_set(inductor, i, j);
          break;
        }

        if (next_b->kind == MU_VARIABLE_TYPE) {
          member_t i = type_member(inductor, next_a);
          member_t j = type_member(inductor, next_b);
          inductor_set(inductor, j, i);
          break;
        }

        if (next_a->kind != next_b->kind) {
          assert(0);
        }

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
    inductor_set(inductor, i, j);
  } while ((type_a = type_return(type_a)) != NULL && (type_b = type_return(type_b)) != NULL);

  return inductor;
}

static member_t node_member(const inductor_t *inductor, const mu_node_t *node) {
  assert(node->as_stator.id < inductor->node_number);
  size_t index = node->as_stator.id;
  return (member_t) { NODE, .index = index, .node = node };
}

static member_t type_member(const inductor_t *inductor, const mu_type_t *type) {
  size_t index = inductor->node_number + type->as_stator.id;
  return (member_t) { TYPE, .index = index, .type = type };
}
