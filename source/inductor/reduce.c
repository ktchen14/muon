#include "reduce.h"
#include "induce.h"

#include "../stator.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

const mu_type_t *reduce_type_result(reduce_t *reduce, const mu_type_t *type);

/// Return the index where the next equivalent type to @a type should be
__attribute__((nonnull, pure))
static inline size_t slot(const reduce_t *reduce, const mu_type_t *type) {
  return type->as_stator.id;
}

/// Get the next type equivalent to @a type in the @a induce context
const mu_type_t *get(const induce_t *induce, const mu_type_t *type)
  __attribute__((nonnull, pure));

const mu_type_t *reduce_type(reduce_t *reduce, const mu_type_t *type) {
  assert(type_cursor(type)->anterior == NULL && type_cursor(type)->i == 0);

  // Return if the type, or a type equivalent to it, is already reduced. We
  // traverse the union-find structure, like get_root, to find each
  // equivalent type. However, at each step, we link the next equivalent type to
  // the previous one using the cursor. When we link an intermediate type like
  // this, the anterior type's cursor will have i == 0. Normally, an anterior
  // type's cursor can't have i == 0 because we always do type_cursor(type)->i++
  // before we type_continue from it.
  for (const mu_type_t *result;;) {
    if ((result = reduce->data[slot(reduce, type)]) != NULL) {
      while ((type = type_return(type)) != NULL)
        reduce->data[slot(reduce, type)] = result;
      return result;
    }

    if ((result = get(reduce->induce, type)) == NULL)
      break;
    type = type_continue(type, result);
  }

  for (const mu_type_t *next, *result;;) {
    while ((next = type_at(type, type_cursor(type)->i++)) != NULL) {
      if (reduce->data[slot(reduce, next)] != NULL)
        continue;

      type = type_continue(type, next);

      while ((next = get(reduce->induce, type)) != NULL)
        type = type_continue(type, next);
    }

    import_t import = {
      .engine = reduce->induce->engine,
      .type = reduce->data,
    };

    if ((result = type_import(type, &import)) == NULL)
      goto except_type_reduce;

    // Unwind each equivalent type and record its reduce result
    do {
      reduce->data[slot(reduce, type)] = result;

      if ((type = type_return(type)) == NULL)
        return result;
    } while (type_cursor(type)->i == 0);
  };

except_type_reduce:
  while ((type = type_return(type)) != NULL);
  return NULL;
}

const mu_type_t *reduce_node(reduce_t *reduce, const mu_node_t *node) {
  return reduce_type(reduce, induce_evince(reduce->induce, node));
}
