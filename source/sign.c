#include "sign.h"

void mu_sign_debug(const mu_sign_t *sign) {
  switch (sign->kind) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_SIGN: \
      mu_##lower##_sign_debug((const mu_##lower##_sign_t *) sign); \
      break;
    MU_EACH_SIGN_KIND(MU_EMIT)
#undef MU_EMIT
  }
}
