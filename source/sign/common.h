#ifndef MU_SIGN_COMMON_I
#define MU_SIGN_COMMON_I

#include <muon/sign/common.h>  // IWYU pragma: export

#include "../node/common.h"

typedef struct {
  const mu_sign_t *a;
  const mu_sign_t *b;
} constraint_t;

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
  size_t length;
  constraint_t data[];
} criteria_t;

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

#endif /* MU_SIGN_COMMON_I */
