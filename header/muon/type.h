#ifndef MU_TYPE_H
#define MU_TYPE_H

#include "type/common.h"   // IWYU pragma: export

#include "type/integer.h"  // IWYU pragma: export
#include "type/vector.h"   // IWYU pragma: export

/// Emit debugging information on the abstract @a type to the debug stream
void mu_type_debug(const mu_type_t *type) __attribute__((nonnull));

#endif /* MU_TYPE_H */
