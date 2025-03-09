#ifndef MU_STATOR_SIGN_H
#define MU_STATOR_SIGN_H

#include "abstract_node.h"

#include "name.h"

#include <stddef.h>

/// The header that each concrete sign must have
#define MU_SIGN_HEADER union { mu_sign_t as_sign; mu_node_t as_node; }

typedef struct {
  MU_SIGN_HEADER;
} mu_boolean_sign_t;

typedef struct {
  MU_SIGN_HEADER;
} mu_integer_sign_t;

typedef struct {
  MU_SIGN_HEADER;
  const mu_name_t *name;
} mu_name_sign_t;

typedef struct {
  const mu_name_t *name; // optional
  const mu_sign_t *sign;
} mu_sign_member_t;

typedef struct {
  MU_SIGN_HEADER;
  size_t argc;
  mu_sign_member_t argv[/* argc */];
} mu_record_sign_t;

typedef struct {
  MU_SIGN_HEADER;
  const mu_sign_t *matter;
} mu_vector_sign_t;

const mu_boolean_sign_t *mu_boolean_sign(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

const mu_integer_sign_t *mu_integer_sign(mu_engine_t *engine)
  __attribute__((malloc, nonnull));

const mu_name_sign_t *mu_name_sign(mu_engine_t *engine, const mu_name_t *name)
  __attribute__((malloc, nonnull));

const mu_record_sign_t *mu_record_sign(
    mu_engine_t *engine, size_t argc, const mu_sign_member_t argv[/* argc */])
  __attribute__((malloc, nonnull(1)));

const mu_vector_sign_t *mu_vector_sign(
    mu_engine_t *engine, const mu_sign_t *matter)
  __attribute__((malloc, nonnull));

/// Emit debugging information on the boolean @a sign to the debug stream
void mu_boolean_sign_debug(const mu_boolean_sign_t *sign)
  __attribute__((nonnull));

/// Emit debugging information on the integer @a sign to the debug stream
void mu_integer_sign_debug(const mu_integer_sign_t *sign)
  __attribute__((nonnull));

/// Emit debugging information on the name @a sign to the debug stream
void mu_name_sign_debug(const mu_name_sign_t *sign)
  __attribute__((nonnull));

/// Emit debugging information on the record @a sign to the debug stream
void mu_record_sign_debug(const mu_record_sign_t *sign)
  __attribute__((nonnull));

/// Emit debugging information on the vector @a sign to the debug stream
void mu_vector_sign_debug(const mu_vector_sign_t *sign)
  __attribute__((nonnull));

#endif /* MU_STATOR_SIGN_H */
