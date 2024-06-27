#ifndef MU_SIGN_H
#define MU_SIGN_H

#include "sign/common.h"   // IWYU pragma: export

#include "sign/integer.h"  // IWYU pragma: export
#include "sign/member.h"   // IWYU pragma: export
#include "sign/record.h"   // IWYU pragma: export
#include "sign/vector.h"   // IWYU pragma: export

void mu_sign_debug(const mu_sign_t *sign) __attribute__((nonnull));

#endif /* MU_SIGN_H */
