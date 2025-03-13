#ifndef MU_INDUCTOR_CORE_I
#define MU_INDUCTOR_CORE_I

#include "../stator/name.h"

#include <stddef.h>

typedef enum {
  MU_BOOLEAN_CORE,
  MU_INTEGER_CORE,
  MU_LAMBDA_CORE,
  MU_VECTOR_CORE,
  MU_RECORD_CORE,
} mu_core_kind_t;

typedef enum {
  MU_COVARIANCE,
  MU_CONTRAVARIANCE,
  MU_INVARIANCE,
} mu_variance_t;

typedef struct {
  const mu_name_t *name;
  mu_variance_t variance;
} mu_core_member_t;

typedef struct {
  mu_core_kind_t kind;
  size_t argc;
  mu_core_member_t argv[/* argc */];
} mu_core_t;

#endif /* MU_INDUCTOR_CORE_I */
