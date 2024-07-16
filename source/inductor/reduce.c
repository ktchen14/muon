#include "reduce.h"

#include "../stator.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef inductor_t induce_t;

const mu_type_t *reduce_type_result(reduce_t *reduce, const mu_type_t *type);

size_t slot(reduce_t *reduce, const mu_type_t *type);

/// Get the next type equivalent to @a type in the @a induce context
const mu_type_t *get(const induce_t *induce, const mu_type_t *type)
  __attribute__((nonnull, pure));

const mu_type_t *evince(const induce_t *induce, const mu_node_t *);

const mu_type_t *reduce_type(reduce_t *reduce_ctx, const mu_type_t *type) {
  assert(type_cursor(type)->anterior == NULL && type_cursor(type)->i == 0);
  const mu_type_t **reduce = reduce_ctx->data;

  // Return if the type, or a type equivalent to it, is already reduced. We
  // traverse the union-find structure, like get_root, to find each
  // equivalent type. However, at each step, we link the next equivalent type to
  // the previous one using the cursor. When we link an intermediate type like
  // this, the anterior type's cursor will have i == 0. Normally, an anterior
  // type's cursor can't have i == 0 because we always do type_cursor(type)->i++
  // before we type_continue from it.
  for (const mu_type_t *result;;) {
    if ((result = reduce[slot(reduce_ctx, type)]) != NULL) {
      while ((type = type_return(type)) != NULL)
        reduce[slot(reduce_ctx, type)] = result;
      return result;
    }

    if ((result = get(reduce_ctx->inductor, type)) == NULL)
      break;
    type = type_continue(type, result);
  }

  for (const mu_type_t *next, *result;;) {
    while ((next = type_at(type, type_cursor(type)->i++)) != NULL) {
      if (reduce[slot(reduce_ctx, next)] != NULL)
        continue;

      type = type_continue(type, next);

      while ((next = get(reduce_ctx->inductor, type)) != NULL)
        type = type_continue(type, next);
    }

    import_t import = {
      .engine = reduce_ctx->inductor->engine,
      .type = reduce_ctx->data,
    };

    if ((result = type_import(type, &import)) == NULL)
      goto except_type_reduce;

    // Unwind each equivalent type and record its reduce result
    do {
      reduce[slot(reduce_ctx, type)] = result;

      if ((type = type_return(type)) == NULL)
        return result;
    } while (type_cursor(type)->i == 0);
  };

except_type_reduce:
  while ((type = type_return(type)) != NULL);
  return NULL;
}

const mu_type_t *reduce_node(reduce_t *reduce, const mu_node_t *node) {
  const mu_type_t *type = evince(reduce->inductor, node);
  assert(type != NULL);
  return reduce_type(reduce, type);
}
