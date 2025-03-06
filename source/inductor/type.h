#ifndef MU_INDUCTOR_TYPE_I
#define MU_INDUCTOR_TYPE_I

#include "core.h"
#include "../stator/name.h"

#include <assert.h>

/// Expands to emit(lower, upper, title, ...) for each kind of type
#define MU_EACH_TYPE_KIND(emit, ...) \
  emit(simple, SIMPLE, Simple, ##__VA_ARGS__) \
  emit(record, RECORD, Record, ##__VA_ARGS__) \
  emit(variable, VARIABLE, Variable, ##__VA_ARGS__) \
  emit(scheme, SCHEME, Scheme, ##__VA_ARGS__)

/// An enumeration over each kind of type
typedef enum {
#define MU_EMIT(l, upper, t) MU_##upper##_TYPE,
  MU_EACH_TYPE_KIND(MU_EMIT)
#undef MU_EMIT
} mu_type_kind_t;

/// An abstract type
typedef struct {
  mu_type_kind_t kind;
} mu_type_t;

/// The header that each concrete type must have
#define MU_TYPE_HEADER mu_type_t as_type

/// A simple type
typedef struct {
  MU_TYPE_HEADER;

  const mu_core_t *core;
  const mu_type_t *argv[/* core->argc */];
} mu_simple_type_t;

/// A record type member
typedef struct {
  const mu_name_t *name;
  const mu_type_t *type;
} mu_type_member_t;

/// A record type
typedef struct {
  size_t argc;
  mu_type_member_t argv[/* argc */];
} mu_record_type_t;

/// A variable type
typedef struct mu_variable_type_t mu_variable_type_t;
struct mu_variable_type_t {
  mu_variable_type_t *scheme_next;

  size_t number; ///< Used to generate a name

  size_t rank;
  const mu_type_t *polymorphic_to;

  _Bool positively_reachable;
  _Bool negatively_reachable;

  const mu_type_t *debug_next;
  const mu_type_t *positively_entered_from;
  const mu_type_t *negatively_entered_from;
};

/// A scheme type
typedef struct {
  const mu_type_t *matter;

  /// Length of list of polymorphic variables
  size_t argc;
  const mu_type_t *argv[/* argc */];
} mu_scheme_type_t;

typedef struct type_t type_t;

typedef struct {
  const mu_name_t *name;
  const type_t *type;
} type_member_t;

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
      type_t *next;

      // Used to generate a name
      size_t number;

      size_t rank;
      const type_t *polymorphic_to;

      _Bool positively_reachable;
      _Bool negatively_reachable;

      const type_t *debug_next;
      const type_t *positively_entered_from;
      const type_t *negatively_entered_from;
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

typedef struct {
  const type_t *next;
} type_link_t;

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
void debug_variable_type_name(const type_t *type);
void debug_just_type(const type_t *type);

/// Compare the type member @a a to the type member @a b
__attribute__((nonnull, pure))
static inline int type_member_cmp(const void *a, const void *b) {
  const type_member_t *ra = a, *rb = b;
  assert(ra->name != NULL && rb->name != NULL);
  return name_cmp(ra->name, rb->name);
}

static inline _Bool is_significant(const type_t *type) {
  return 1;
  assert(type->kind == VARIABLE_TYPE);
  return type->positively_entered_from == type && type->negatively_entered_from == type
    || type->polymorphic_to != NULL;
}

#endif /* MU_INDUCTOR_TYPE_I */
