#ifndef MU_INDUCTOR_I
#define MU_INDUCTOR_I

#include "type.h"

// TODO
#include <muon/node.h>

#include <assert.h>
#include <stddef.h>

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

  size_t node_length;
  size_t type_length;

  // Length = node_length + type_length
  inductor_member_t *data;
};

/// Initialize the @a inductor
inductor_t *inductor_initialize(inductor_t *inductor, mu_engine_t *engine)
  __attribute__((nonnull));

const inductor_member_t *inductor_get(
    const inductor_t *inductor, const inductor_member_t *member)
  __attribute__((nonnull, pure, returns_nonnull));

const inductor_member_t *inductor_set(
    inductor_t *inductor,
    const inductor_member_t *restrict source,
    const inductor_member_t *restrict target)
  __attribute__((nonnull));

const inductor_member_t *inductor_root(
    inductor_t *inductor, const inductor_member_t *member)
  __attribute__((nonnull));

__attribute__((nonnull))
static inline const mu_type_t *inductor_type_root(
    inductor_t *inductor, const mu_type_t *type) {
  inductor_member_t member = {
    INDUCTOR_TYPE, .id = type->as_stator.id, .type = type,
  };

  const inductor_member_t *result;
  result = inductor_root(inductor, &member);
  assert(result->kind == INDUCTOR_TYPE);

  return result->type;
}

const mu_type_t *inductor_type(inductor_t *inductor, const mu_node_t *node);

inductor_t *inductor_equate_node_node(
    inductor_t *inductor, const mu_node_t *a, const mu_node_t *b);

inductor_t *inductor_equate_node_type(
    inductor_t *inductor, const mu_node_t *a, const mu_type_t *b);

inductor_t *inductor_equate_type_type(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b);

#endif /* MU_INDUCTOR_I */
