#include "common.h"
#include "engine.h"
#include "inductor.h"
#include "type.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef inductor_member_t member_t;

static const member_t member_none = {0};

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

const inductor_member_t *inductor_root(
    inductor_t *inductor, const inductor_member_t *member) {
  for (;;) {
    const inductor_member_t *next = inductor_get(inductor, member);
    if (next->kind == INDUCTOR_NONE)
      return member;
    member = next;
  }

  return member;
}

const member_t *inductor_set(
    inductor_t *inductor, const member_t *source, const member_t *target) {
  if (source->kind == INDUCTOR_NODE) {
    assert(source->id < inductor->node_length);
    inductor->data[source->id] = *target;
    return target;
  }

  if (source->id >= inductor->type_length) {
    fprintf(stderr, "Reallocating\n");
    size_t origin = inductor->node_length + inductor->type_length;
    size_t length = inductor->node_length + source->id + 200;

    member_t *data = inductor->data;
    if ((data = realloc(data, sizeof(member_t[length]))) == NULL)
      return NULL;
    for (size_t i = origin; i < length; data[i++] = (member_t) {0});

    inductor->type_length = source->id + 200;
    inductor->data = data;
  }

  inductor->data[inductor->node_length + source->id] = *target;
  return target;
}

const mu_type_t *inductor_type(inductor_t *inductor, const mu_node_t *node) {
  member_t member = {
    .kind = INDUCTOR_NODE, .id = node->as_stator.id, .node = node,
  };
  const member_t *root_member = inductor_root(inductor, &member);

  // TODO: create a type variable
  if (root_member->kind == INDUCTOR_NODE)
    return NULL;

  const mu_type_t *type = root_member->type;

  do {
    const mu_type_t *next;
    while ((next = type_at(type, type_cursor(type)->i++)) != NULL) {
      const member_t *result;
      member = (member_t) {
        .kind = INDUCTOR_TYPE, .id = next->as_stator.id, .type = next,
      };
      result = inductor_root(inductor, &member);
      assert(result->kind == INDUCTOR_TYPE);
      type = type_continue(type, result->type);
    }

    const mu_type_t *result = type_reduce(type, inductor);
    if (result != type) {
      member_t i = { INDUCTOR_TYPE, .id = type->as_stator.id, .type = type };
      member_t j = { INDUCTOR_TYPE, .id = result->as_stator.id, .type = result };
      inductor_set(inductor, &i, &j);
    }
  } while ((type = type_return(type)) != NULL);

  member = (member_t) {
    .kind = INDUCTOR_NODE, .id = node->as_stator.id, .node = node,
  };
  root_member = inductor_root(inductor, &member);
  assert(root_member->kind == INDUCTOR_TYPE);
  return root_member->type;
}

static inductor_t *inductor_unify(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b) {
  // If a and b are already unified then we're done
  if (a == b)
    return inductor;

  // If a is a variable type, then just equate it to b
  if (a->kind == MU_VARIABLE_TYPE) {
    member_t i = { INDUCTOR_TYPE, .id = a->as_stator.id, .type = a };
    member_t j = { INDUCTOR_TYPE, .id = b->as_stator.id, .type = b };
    inductor_set(inductor, &i, &j);
    return inductor;
  }

  if (b->kind == MU_VARIABLE_TYPE) {
    member_t i = { INDUCTOR_TYPE, .id = a->as_stator.id, .type = a };
    member_t j = { INDUCTOR_TYPE, .id = b->as_stator.id, .type = b };
    inductor_set(inductor, &j, &i);
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
          member_t i = { INDUCTOR_TYPE, .id = next_a->as_stator.id, .type = next_a };
          member_t j = { INDUCTOR_TYPE, .id = next_b->as_stator.id, .type = next_b };
          inductor_set(inductor, &i, &j);
          break;
        }

        if (next_b->kind == MU_VARIABLE_TYPE) {
          member_t i = { INDUCTOR_TYPE, .id = next_a->as_stator.id, .type = next_a };
          member_t j = { INDUCTOR_TYPE, .id = next_b->as_stator.id, .type = next_b };
          inductor_set(inductor, &j, &i);
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
    member_t i = { INDUCTOR_TYPE, .id = type_a->as_stator.id, .type = type_a };
    member_t j = { INDUCTOR_TYPE, .id = type_b->as_stator.id, .type = type_b };
    inductor_set(inductor, &i, &j);
  } while ((type_a = type_return(type_a)) != NULL && (type_b = type_return(type_b)) != NULL);

  return inductor;
}

static inductor_t *inductor_equate(
    inductor_t *inductor, const member_t *a, const member_t *b) {
  a = inductor_root(inductor, a);
  b = inductor_root(inductor, b);

  if (a == b)
    return inductor;

  if (a->kind == INDUCTOR_NODE && b->kind == INDUCTOR_NODE) {
    fprintf(stderr, "Equating node %zu to node %zu\n",
        a->node->as_stator.id,
        b->node->as_stator.id);

    inductor_set(inductor, a, b);
  } else if (a->kind == INDUCTOR_NODE && b->kind == INDUCTOR_TYPE) {
    fprintf(stderr, "Equating node %zu to type ",
        a->node->as_stator.id);
    mu_type_debug(b->type);
    fprintf(stderr, "\n");

    inductor_set(inductor, a, b);
  } else if (a->kind == INDUCTOR_TYPE && b->kind == INDUCTOR_NODE) {
    fprintf(stderr, "Equating node %zu to type ",
        b->node->as_stator.id);
    mu_type_debug(a->type);
    fprintf(stderr, "\n");

    inductor_set(inductor, b, a);
  } else {
    fprintf(stderr, "Equating type ");
    mu_type_debug(a->type);
    fprintf(stderr, " to type ");
    mu_type_debug(b->type);
    fprintf(stderr, "\n");

    return inductor_unify(inductor, a->type, b->type);
  }

  return inductor;
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
