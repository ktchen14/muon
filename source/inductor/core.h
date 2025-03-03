#ifndef MU_INDUCTOR_CORE_I
#define MU_INDUCTOR_CORE_I

#include <stddef.h>

typedef enum {
  MU_BOOLEAN_CORE,
  MU_INTEGER_CORE,
  MU_LAMBDA_CORE,
  MU_VECTOR_CORE,
} mu_core_kind_t;

typedef enum {
  MU_COVARIANCE,
  MU_CONTRAVARIANCE,
  MU_INVARIANCE,
} mu_variance_t;

typedef struct {
  mu_core_kind_t kind;
  size_t argc;
  mu_variance_t variance[/* argc */];
} mu_core_t;

#endif /* MU_INDUCTOR_CORE_I */
