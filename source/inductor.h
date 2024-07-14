#ifndef MU_INDUCTOR_I
#define MU_INDUCTOR_I

#include <muon/stator.h>

#include "status.h"

#include <assert.h>
#include <stddef.h>

typedef struct inductor_member_t inductor_member_t;
typedef struct inductor_t inductor_t;

struct inductor_t {
  mu_engine_t *engine;

  size_t node_number;
  size_t length;
  inductor_member_t *data;

  const mu_stmt_t *const *node_to_stmt;

  mu_status_t *status;
};

/// Initialize the @a inductor to handle nodes and types in the @a engine
inductor_t *inductor_initialize(
    inductor_t *inductor,
    mu_engine_t *engine,
    const mu_stmt_t *const *node_to_stmt,
    mu_status_t *status)
  __attribute__((nonnull));

void inductor_raze(inductor_t *inductor) __attribute__((nonnull));

const mu_type_t *inductor_type_root(
    inductor_t *inductor, const mu_type_t *type)
  __attribute__((nonnull));

/// Return the archtype of the @a type in the @a inductor
const mu_type_t *inductor_type_of_type(
    inductor_t *inductor, const mu_type_t *type)
  __attribute__((nonnull));

/// Return the archtype of the @a node in the @a inductor
const mu_type_t *inductor_type_of_node(
    inductor_t *inductor, const mu_node_t *node)
  __attribute__((nonnull));

/// Equate node @a a to node @a b in the @a inductor
inductor_t *inductor_equate_node_node(
    inductor_t *inductor, const mu_node_t *a, const mu_node_t *b)
  __attribute__((nonnull));

/// Equate node @a a to type @a b in the @a inductor
inductor_t *inductor_equate_node_type(
    inductor_t *inductor, const mu_node_t *a, const mu_type_t *b)
  __attribute__((nonnull));

/// Equate type @a a to type @a b in the @a inductor
inductor_t *inductor_equate_type_type(
    inductor_t *inductor, const mu_type_t *a, const mu_type_t *b)
  __attribute__((nonnull));

#endif /* MU_INDUCTOR_I */
