#include "common.h"
#include "engine.h"
#include "inductor.h"
#include "type.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

/* const mu_type_t *type_equate( */
/*     inductor_t *inductor, const mu_type_t *a, const mu_type_t *b) */
/*   __attribute__((nonnull)); */

inductor_t *type_equate(
    const mu_type_t *a,
    const mu_type_t *b,
    inductor_t *inductor);

inductor_t *inductor_create(mu_engine_t *engine) {
  size_t length = engine->stator_id;

  size_t size;
  if (rare((size = struct_size(inductor_t, data, length)) == 0))
    return NULL;

  inductor_t *inductor;
  if ((inductor = malloc(size)) == NULL)
    return NULL;
  *inductor = (inductor_t) { .engine = engine, .length = length };

  for (size_t i = 0; i < length; i++)
    inductor->data[i] = (inductor_member_t) {0};

  return inductor;
}

inductor_t *inductor_reallocate(
    inductor_t *inductor, const mu_type_t *type) {
  size_t length = type->as_stator.id + 1;

  size_t size;
  if (rare((size = struct_size(inductor_t, data, length)) == 0))
    return NULL;

  if ((inductor = realloc(inductor, size)) == NULL)
    return NULL;

  for (size_t i = inductor->length; i < length; i++)
    inductor->data[i] = (inductor_member_t) {0};

  inductor->length = length;

  return inductor;
}

inductor_t *inductor_unify(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b) {
  // If a and b are already unified then we're done
  if (a == b)
    return inductor;

  // Swap a and b if b is a variable type
  if (b->kind == MU_VARIABLE_TYPE) {
    const mu_type_t *t = a;
    a = b;
    b = t;
  }

  // If a is a variable type, then just equate it to b
  if (a->kind == MU_VARIABLE_TYPE) {
    inductor->data[a->as_stator.id] = (inductor_member_t) {
      .kind = INDUCTOR_TYPE_MEMBER,
      .id = b->as_stator.id,
      .stator = &b->as_stator };
    return inductor;
  }

  if (b->kind == MU_VARIABLE_TYPE) {
    inductor->data[b->as_stator.id] = (inductor_member_t) {
      .kind = INDUCTOR_TYPE_MEMBER,
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
          inductor->data[next_a->as_stator.id] = (inductor_member_t) {
            .kind = INDUCTOR_TYPE_MEMBER,
            .id = next_b->as_stator.id,
            .stator = &next_b->as_stator };
          break;
        }

        if (next_b->kind == MU_VARIABLE_TYPE) {
          inductor->data[next_b->as_stator.id] = (inductor_member_t) {
            .kind = INDUCTOR_TYPE_MEMBER,
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
    inductor->data[type_a->as_stator.id] = (inductor_member_t) {
      .kind = INDUCTOR_TYPE_MEMBER,
      .id = type_b->as_stator.id,
      .stator = &type_b->as_stator };
  } while ((type_a = type_return(type_a)) != NULL && (type_b = type_return(type_b)) != NULL);

  return inductor;
}
