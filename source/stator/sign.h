#ifndef MU_STATOR_SIGN_I
#define MU_STATOR_SIGN_I

#include <muon/stator/sign.h>  // IWYU pragma: export

#include <stddef.h>

/// Return the <em>i</em>th node in the boolean @a sign
__attribute__((const, nonnull))
static inline const mu_node_t *boolean_sign_at(
    const mu_boolean_sign_t *sign, size_t i) {
  return NULL;
}

/// Return the <em>i</em>th node in the integer @a sign
__attribute__((const, nonnull))
static inline const mu_node_t *integer_sign_at(
    const mu_integer_sign_t *sign, size_t i) {
  return NULL;
}

/// Return the <em>i</em>th node in the name @a sign
__attribute__((const, nonnull))
static inline const mu_node_t *name_sign_at(
    const mu_name_sign_t *sign, size_t i) {
  return NULL;
}

/// Return the <em>i</em>th node in the record @a sign
__attribute__((nonnull, pure))
static inline const mu_node_t *record_sign_at(
    const mu_record_sign_t *sign, size_t i) {
  return i < sign->argc ? &sign->argv[i].sign->as_node : NULL;
}

/// Return the <em>i</em>th node in the vector @a sign
__attribute__((nonnull, pure))
static inline const mu_node_t *vector_sign_at(
    const mu_vector_sign_t *sign, size_t i) {
  return i == 0 ? &sign->matter->as_node : NULL;
}

#endif /* MU_STATOR_SIGN_I */
