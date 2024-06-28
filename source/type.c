#include "type.h"

void mu_type_debug(const mu_type_t *type) {
  switch (type->kind) { MU_EACH_TYPE_KIND(MU_ABSTRACT_TYPE_CALL, mu, debug) }
}
