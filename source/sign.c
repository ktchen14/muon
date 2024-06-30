#include "sign.h"

void mu_sign_debug(const mu_sign_t *sign) {
  switch (sign->kind) { MU_EACH_SIGN_KIND(MU_ABSTRACT_SIGN_CALL, sign, mu, debug) }
}
