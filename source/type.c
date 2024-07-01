#include "type.h"

void mu_type_debug(const mu_type_t *type) {
  switch (type->kind) {
#define MU_EMIT(lower, upper, _) \
    case MU_##upper##_TYPE: \
      mu_##lower##_type_debug((const mu_##lower##_type_t *) type); \
      break;
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
  }
}
