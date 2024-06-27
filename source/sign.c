#include "sign.h"

void mu_sign_debug(const mu_sign_t *sign) {
  switch (sign->kind) {
    case MU_INTEGER_SIGN:
      mu_integer_sign_debug((const mu_integer_sign_t *) sign);
      break;

    case MU_MEMBER_SIGN:
      mu_member_sign_debug((const mu_member_sign_t *) sign);
      break;

    case MU_NAME_SIGN:
      mu_name_sign_debug((const mu_name_sign_t *) sign);
      break;

    case MU_RECORD_SIGN:
      mu_record_sign_debug((const mu_record_sign_t *) sign);
      break;

    case MU_VARIABLE_SIGN:
      mu_variable_sign_debug((const mu_variable_sign_t *) sign);
      break;

    case MU_VECTOR_SIGN:
      mu_vector_sign_debug((const mu_vector_sign_t *) sign);
      break;
  }
}
