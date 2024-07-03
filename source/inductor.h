#ifndef MU_INDUCTOR_I
#define MU_INDUCTOR_I

#include "engine.h"
#include "stmt.h"
#include "type.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
  /// Whether the target stator is a node or type
  enum {
    INDUCTOR_NONE, INDUCTOR_NODE_MEMBER, INDUCTOR_TYPE_MEMBER,
  } kind;
  size_t id;
  const mu_stator_t *stator;
} inductor_member_t;

typedef struct inductor_t inductor_t;

struct inductor_t {
  mu_engine_t *engine;
  const mu_stmt_t *const *node_to_stmt;
  size_t length;
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

static inline inductor_member_t *inductor_root(
    inductor_t *inductor, inductor_member_t *member) {
  for (;;) {
    inductor_member_t *next = &inductor->data[member->id];
    if (next->kind == INDUCTOR_NONE)
      return member;
    member = next;
  }

  return member;
}

const mu_type_t *inductor_type(
    inductor_t **inductor, const mu_node_t *node);

inductor_t *inductor_unify(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b);

static inline inductor_t *inductor_equate(
    inductor_t *inductor, inductor_member_t *a, inductor_member_t *b) {
  a = inductor_root(inductor, a);
  b = inductor_root(inductor, b);

  if (a == b)
    return inductor;

  if (a->kind == INDUCTOR_NODE_MEMBER && b->kind == INDUCTOR_NODE_MEMBER) {
    inductor->data[a->id] = *b;
  } else if (a->kind == INDUCTOR_NODE_MEMBER && b->kind == INDUCTOR_TYPE_MEMBER) {
    inductor->data[a->id] = *b;
  } else if (a->kind == INDUCTOR_TYPE_MEMBER && b->kind == INDUCTOR_NODE_MEMBER) {
    inductor->data[b->id] = *a;
  } else
    return inductor_unify(inductor, (const mu_type_t *) a->stator, (const mu_type_t *) b->stator);

  return inductor;
}

static inline inductor_t *inductor_equate_node_node(
    inductor_t *inductor, const mu_node_t *a, const mu_node_t *b) {
  inductor_member_t i = {
    .kind = INDUCTOR_NODE_MEMBER,
    .id = a->as_stator.id,
    .stator = &a->as_stator };
  inductor_member_t j = {
    .kind = INDUCTOR_NODE_MEMBER,
    .id = b->as_stator.id,
    .stator = &b->as_stator };
  return inductor_equate(inductor, &i, &j);
}

static inline inductor_t *inductor_equate_node_type(
    inductor_t *inductor, const mu_node_t *a, const mu_type_t *b) {
  inductor_member_t i = {
    .kind = INDUCTOR_NODE_MEMBER,
    .id = a->as_stator.id,
    .stator = &a->as_stator };
  inductor_member_t j = {
    .kind = INDUCTOR_TYPE_MEMBER,
    .id = b->as_stator.id,
    .stator = &b->as_stator };
  return inductor_equate(inductor, &i, &j);
}

static inline inductor_t *inductor_equate_type_type(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b) {
  inductor_member_t i = {
    .kind = INDUCTOR_TYPE_MEMBER,
    .id = a->as_stator.id,
    .stator = &a->as_stator };
  inductor_member_t j = {
    .kind = INDUCTOR_TYPE_MEMBER,
    .id = b->as_stator.id,
    .stator = &b->as_stator };
  return inductor_equate(inductor, &i, &j);
}

#endif /* MU_INDUCTOR_I */
