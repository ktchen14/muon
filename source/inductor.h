#ifndef MU_INDUCTOR_I
#define MU_INDUCTOR_I

#include "type.h"

// TODO
#include <muon/node.h>

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
  /// Whether the target stator is a node or type
  enum {
    INDUCTOR_NONE, INDUCTOR_NODE, INDUCTOR_TYPE,
  } kind;
  size_t id;
  union {
    const mu_stator_t *stator;
    const mu_node_t *node;
    const mu_type_t *type;
  };
} inductor_member_t;

typedef struct inductor_t inductor_t;

struct inductor_t {
  mu_engine_t *engine;
  const mu_stmt_t *const *node_to_stmt;
  size_t length;

  size_t node_length;
  size_t type_length;

  inductor_member_t data[];
};

/**
 * @brief Create an inductor on the @a engine
 */
inductor_t *inductor_create(mu_engine_t *engine)
  __attribute__((nonnull));

/// @internal Extend the @a inductor so that it's able to hold the @a type
inductor_t *inductor_reallocate(inductor_t *inductor, const mu_type_t *type)
  __attribute__((nonnull));

static inline inductor_t *inductor_extend_type(
    inductor_t *inductor, const mu_type_t *type) {
  if (type->as_stator.id < inductor->length)
    return inductor;
  return inductor_reallocate(inductor, type);
}

const inductor_member_t *inductor_get(
    const inductor_t *inductor, const inductor_member_t *member);

static inline inductor_member_t *inductor_root(
    inductor_t *inductor, inductor_member_t *member) {
  for (;;) {
    inductor_member_t *next = (inductor_member_t *) inductor_get(inductor, member);
    if (next->kind == INDUCTOR_NONE)
      return member;
    member = next;
  }

  return member;
}

const mu_type_t *inductor_type(
    inductor_t **inductor, const mu_node_t *node);

inductor_t *inductor_equate_node_node(
    inductor_t *inductor, const mu_node_t *a, const mu_node_t *b);

inductor_t *inductor_equate_node_type(
    inductor_t *inductor, const mu_node_t *a, const mu_type_t *b);

inductor_t *inductor_equate_type_type(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b);

#endif /* MU_INDUCTOR_I */
