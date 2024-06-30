#ifndef MU_SIGN_I
#define MU_SIGN_I

#include <muon/sign.h>      // IWYU pragma: export

#include "sign/integer.h"   // IWYU pragma: export
#include "sign/member.h"    // IWYU pragma: export
#include "sign/name.h"      // IWYU pragma: export
#include "sign/record.h"    // IWYU pragma: export
#include "sign/variable.h"  // IWYU pragma: export
#include "sign/vector.h"    // IWYU pragma: export

/**
 * @brief Emit a @c case in an abstract call on an sign function
 */
#define MU_ABSTRACT_SIGN_CALL(lower, upper, t, variable, prefix, name, ...) \
  case MU_##upper##_SIGN: \
    prefix##_##lower##_sign_##name((const mu_##lower##_sign_t *) sign, ##__VA_ARGS__); \
    break;

#endif /* MU_SIGN_I */
