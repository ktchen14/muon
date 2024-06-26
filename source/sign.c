#include "sign.h"

void mu_sign_debug(const mu_sign_t *sign) {
  switch (sign->kind) {
    case MU_INTEGER_SIGN:
      return mu_integer_sign_debug((const mu_integer_sign_t *) sign);

    case MU_MEMBER_SIGN:
      return mu_member_sign_debug((const mu_member_sign_t *) sign);

    case MU_NAME_SIGN:
      return mu_name_sign_debug((const mu_name_sign_t *) sign);

    case MU_RECORD_SIGN:
      return mu_record_sign_debug((const mu_record_sign_t *) sign);

    case MU_VECTOR_SIGN:
      return mu_vector_sign_debug((const mu_vector_sign_t *) sign);
  }
}
