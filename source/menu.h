#ifndef MU_MENU_I
#define MU_MENU_I

#include "stmt.h"
#include "type.h"

#include <errno.h>
#include <stdlib.h>

typedef struct {
  mu_engine_t *engine;
  const mu_type_t *const *node_to_type;
  const mu_stmt_t *const *node_to_stmt;
} induce_t;

typedef struct {
  union {
    const mu_node_t *node;
    const mu_type_t *type;
  } a;

  union {
    const mu_node_t *node;
    const mu_type_t *type;
  } b;
} constraint_t;

typedef struct {
  size_t length;
  constraint_t data[];
} criteria_t;

static inline criteria_t *criteria_append(
    criteria_t *criteria, constraint_t constraint) {
  size_t length;
  if (rare(__builtin_add_overflow(criteria->length, 1, &length)))
    return errno = ENOMEM, NULL;

  size_t size;
  if (rare((size = struct_size(criteria_t, data, length)) == 0))
    return errno = ENOMEM, NULL;

  criteria_t *result;
  if (rare((result = realloc(criteria, size)) == NULL))
    return NULL;

  result->data[result->length++] = constraint;
  return result;
}

static inline criteria_t *criteria_extend(criteria_t *criteria, size_t length) {
  if (rare(__builtin_add_overflow(criteria->length, length, &length)))
    return errno = ENOMEM, NULL;

  size_t size;
  if (rare((size = struct_size(criteria_t, data, length)) == 0))
    return errno = ENOMEM, NULL;

  criteria_t *result;
  if (rare((result = realloc(criteria, size)) == NULL))
    return NULL;

  for (size_t i = result->length; i < length; i++)
    result->data[i] = (constraint_t) {0};
  result->length = length;

  return result;
}

#endif /* MU_MENU_I */
