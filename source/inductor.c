#include "common.h"
#include "engine.h"
#include "inductor.h"
#include "type.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef inductor_member_t member_t;

inductor_t *inductor_create(mu_engine_t *engine) {
  size_t length = engine->node_number + engine->type_number;

  size_t size;
  if (rare((size = struct_size(inductor_t, data, length)) == 0))
    return NULL;

  inductor_t *inductor;
  if ((inductor = malloc(size)) == NULL)
    return NULL;
  *inductor = (inductor_t) {
    .engine = engine,
    .length = length,
    .node_length = engine->node_number,
    .type_length = engine->type_number,
  };

  for (size_t i = 0; i < length; i++)
    inductor->data[i] = (member_t) {0};

  return inductor;
}

inductor_t *inductor_reallocate(
    inductor_t *inductor, const mu_type_t *type) {
  size_t length = type->as_stator.id + 1;
  length = 1000;

  size_t size;
  if (rare((size = struct_size(inductor_t, data, length)) == 0))
    return NULL;

  if ((inductor = realloc(inductor, size)) == NULL)
    return NULL;

  for (size_t i = inductor->length; i < length; i++)
    inductor->data[i] = (member_t) {0};

  inductor->length = length;

  return inductor;
}

const mu_type_t *inductor_type(
    inductor_t **inductor, const mu_node_t *node) {
  size_t length = (*inductor)->engine->node_number + (*inductor)->engine->type_number;
  const mu_type_t **equation;
  equation = malloc(sizeof(const mu_type_t *[length]));
  assert(equation != NULL);
  for (size_t i = 0; i < length; i++)
    equation[i] = NULL;

  member_t member = {
    .kind = INDUCTOR_NODE,
    .id = node->as_stator.id,
    .stator = &node->as_stator };

  member_t *root_member = inductor_root(*inductor, &member);

  // TODO: create a type variable
  if (root_member->kind == INDUCTOR_NODE)
    return NULL;

  const mu_type_t *root = (const mu_type_t *) root_member->stator;
  const mu_type_t *type = root;

  do {
    const mu_type_t *next;
    while ((next = type_at(type, type_cursor(type)->i++)) != NULL) {
      member = (member_t) {
        .kind = INDUCTOR_TYPE,
        .id = next->as_stator.id,
        .stator = &next->as_stator };
      root_member = inductor_root(*inductor, &member);
      assert(root_member->kind == INDUCTOR_TYPE);

      next = (const mu_type_t *) root_member->stator;
      type = type_continue(type, next);
    }

    const mu_type_t *result = type_reduce(type, (*inductor)->engine, *inductor);
    if (result != type) {
      (*inductor)->data[type->as_stator.id] = (member_t) {
        .kind = INDUCTOR_TYPE,
        .id = result->as_stator.id,
        .stator = &result->as_stator };
    }
    equation[type->as_stator.id] = result;
  } while ((type = type_return(type)) != NULL);

  member = (member_t) {
    .kind = INDUCTOR_NODE,
    .id = node->as_stator.id,
    .stator = &node->as_stator };

  root_member = inductor_root(*inductor, &member);
  assert(root_member->kind == INDUCTOR_TYPE);
  return (const mu_type_t *) root_member->stator;
}

static inductor_t *inductor_unify(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b) {
  // If a and b are already unified then we're done
  if (a == b)
    return inductor;

  // If a is a variable type, then just equate it to b
  if (a->kind == MU_VARIABLE_TYPE) {
    inductor->data[a->as_stator.id] = (member_t) {
      .kind = INDUCTOR_TYPE,
      .id = b->as_stator.id,
      .stator = &b->as_stator };
    return inductor;
  }

  if (b->kind == MU_VARIABLE_TYPE) {
    inductor->data[b->as_stator.id] = (member_t) {
      .kind = INDUCTOR_TYPE,
      .id = a->as_stator.id,
      .stator = &a->as_stator };
    return inductor;
  }

  // Otherwise, both a and b are concrete types. If they don't have the same
  // kind, then they can't be unified.
  if (a->kind != b->kind)
    assert(0);

  // Unify each type within a and b
  const mu_type_t *type_a = a, *type_b = b;

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
          inductor->data[next_a->as_stator.id] = (member_t) {
            .kind = INDUCTOR_TYPE,
            .id = next_b->as_stator.id,
            .stator = &next_b->as_stator };
          break;
        }

        if (next_b->kind == MU_VARIABLE_TYPE) {
          inductor->data[next_b->as_stator.id] = (member_t) {
            .kind = INDUCTOR_TYPE,
            .id = next_a->as_stator.id,
            .stator = &next_a->as_stator };
          break;
        }

        if (a->kind != b->kind)
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
    inductor->data[type_a->as_stator.id] = (member_t) {
      .kind = INDUCTOR_TYPE,
      .id = type_b->as_stator.id,
      .stator = &type_b->as_stator };
  } while ((type_a = type_return(type_a)) != NULL && (type_b = type_return(type_b)) != NULL);

  return inductor;
}

static inductor_t *inductor_equate(
    inductor_t *inductor, member_t *a, member_t *b) {
  a = inductor_root(inductor, a);
  b = inductor_root(inductor, b);

  if (a == b)
    return inductor;

  if (a->kind == INDUCTOR_NODE && b->kind == INDUCTOR_NODE) {
    inductor->data[a->id] = *b;
  } else if (a->kind == INDUCTOR_NODE && b->kind == INDUCTOR_TYPE) {
    inductor->data[a->id] = *b;
  } else if (a->kind == INDUCTOR_TYPE && b->kind == INDUCTOR_NODE) {
    inductor->data[b->id] = *a;
  } else
    return inductor_unify(inductor, a->type, b->type);

  return inductor;
}

static const member_t member_none = {0};

__attribute__((nonnull, pure, returns_nonnull))
const member_t *inductor_get(
    const inductor_t *inductor, const member_t *member) {
  if (member->kind == INDUCTOR_NODE) {
    assert(member->id < inductor->node_length);
    return &inductor->data[member->id];
  }

  if (member->id >= inductor->type_length)
    return &member_none;

  return &inductor->data[inductor->node_length + member->id];
}

static inline const member_t *inductor_roota(
    const inductor_t *inductor, const member_t *member) {
  const member_t *next;
  while ((next = inductor_get(inductor, member))->stator != NULL) {
    member = next;
  }
  return member;
}

inductor_t *inductor_equate_node_node(
    inductor_t *inductor, const mu_node_t *a, const mu_node_t *b) {
  member_t i = { INDUCTOR_NODE, .id = a->as_stator.id, .node = a };
  member_t j = { INDUCTOR_NODE, .id = b->as_stator.id, .node = b };
  return inductor_equate(inductor, &i, &j);
}

inductor_t *inductor_equate_node_type(
    inductor_t *inductor, const mu_node_t *a, const mu_type_t *b) {
  member_t i = { INDUCTOR_NODE, .id = a->as_stator.id, .node = a };
  member_t j = { INDUCTOR_TYPE, .id = b->as_stator.id, .type = b };
  return inductor_equate(inductor, &i, &j);
}

inductor_t *inductor_equate_type_type(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b) {
  member_t i = { INDUCTOR_TYPE, .id = a->as_stator.id, .type = a };
  member_t j = { INDUCTOR_TYPE, .id = b->as_stator.id, .type = b };
  return inductor_equate(inductor, &i, &j);
}
