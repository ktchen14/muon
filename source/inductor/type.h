#ifndef MU_INDUCTOR_TYPE_I
#define MU_INDUCTOR_TYPE_I

#include "core.h"
#include "../stator.h"

typedef struct type_t type_t;

typedef struct {
  const mu_name_t *name;
  const type_t *type;
} type_member_t;

/// Compare the type member @a a to the type member @a b
__attribute__((nonnull, pure))
static inline int type_member_cmp(const void *a, const void *b) {
  const type_member_t *ra = a, *rb = b;
  assert(ra->name != NULL && rb->name != NULL);
  return name_cmp(ra->name, rb->name);
}

struct type_t {
  enum {
    SIMPLE_TYPE,
    RECORD_TYPE,
    VARIABLE_TYPE,
    SCHEME_TYPE,
    JOIN_TYPE,
  } kind;

  union {
    // SIMPLE_TYPE
    struct {
      const mu_core_t *core;
      const type_t *argv[/* core->argc */];
    };

    // RECORD_TYPE
    struct {
      size_t argc;
      type_member_t schema[];
    };

    // VARIABLE_TYPE
    struct {
      const type_t *next;

      // Used to generate a name
      size_t number;

      size_t rank;
      const type_t *polymorphic_to;

      _Bool positively_reachable;
      _Bool negatively_reachable;
    };

    // SCHEME_TYPE
    struct {
      const type_t *matter;
      size_t polymorphic_length;
      const type_t *polymorphic[];
    };

    // JOIN_TYPE
    struct {
      size_t join_argc;
      const type_t *join_argv[];
    };
  };
};

typedef struct induce_t induce_t;

const type_t *boolean_type(induce_t *induce);
const type_t *integer_type(induce_t *induce);

const type_t *lambda_type(induce_t *induce, const type_t *argument, const type_t *output);

const type_t *record_type(
    induce_t *induce, size_t argc, const type_member_t argv[static argc]);

type_t *record_type_allocate(induce_t *induce, size_t argc);

const type_t *record_type_activate(type_t *type);

const type_t *vector_type(induce_t *induce, const type_t *matter);
const type_t *join_type(induce_t *induce, size_t argc, const type_t *argv[]);
type_t *join_type_allocate(induce_t *induce, size_t argc);
const type_t *join_type_activate(type_t *type);

void debug_type(const type_t *type);

#endif /* MU_INDUCTOR_TYPE_I */
