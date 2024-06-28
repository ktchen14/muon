#ifndef MU_TYPE_I
#define MU_TYPE_I

#include <muon/type.h>      // IWYU pragma: export

#include "type/integer.h"   // IWYU pragma: export
#include "type/vector.h"    // IWYU pragma: export

/**
 * @brief Emit a @c case in an abstract call on an type function
 */
#define MU_ABSTRACT_TYPE_CALL(lower, upper, t, prefix, name, ...) \
  case MU_##upper##_TYPE: \
    prefix##_##lower##_type_##name((const mu_##lower##_type_t *) type, ##__VA_ARGS__); \
    break;

#endif /* MU_TYPE_I */
