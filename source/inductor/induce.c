#include "induce.h"

#include "detect.h"
#include "../stator.h"
#include "../status.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Thread_local induce_t *debug_induce;

const type_t *variable_type(induce_t *induce, open_scheme_t *scheme) {
  type_t *result;
  if ((result = malloc(sizeof(type_t))) == NULL)
    return NULL;

  *result = (type_t) {
    .kind = VARIABLE_TYPE, .rank = scheme->rank, .next = scheme->link,
  };
  scheme->link = result;
  return result;
}

/// Register @a a <: @a b in the @a induce engine
static const type_t *append(
    induce_t *induce, const type_t *restrict a, const type_t *restrict b)
  __attribute__((nonnull));

typedef struct {
  const type_t *source;
  const type_t *target;
} cache_item;

const type_t *instantiate_single_type(
    induce_t *induce,
    const type_t *type,
    const type_t *scheme,
    open_scheme_t *target_scheme,
    cache_item *cache,
    size_t *cache_i
) {
  for (size_t i = 0; i < 100; i++) {
    if (cache[i].source == type)
      return cache[i].target;
  }

  switch (type->kind) {
    case SIMPLE_TYPE:
      switch (type->core->kind) {
        case MU_BOOLEAN_CORE:
        case MU_INTEGER_CORE:
          cache[(*cache_i)++] = (cache_item) { type, type };
          return type;

        case MU_LAMBDA_CORE:
        {
          const type_t *argv_0 = instantiate_single_type(induce, type->argv[0], scheme, target_scheme, cache, cache_i);
          const type_t *argv_1 = instantiate_single_type(induce, type->argv[1], scheme, target_scheme, cache, cache_i);

          const type_t *result = type;
          if (argv_0 != type->argv[0] || argv_1 == type->argv[1])
            result = lambda_type(induce, argv_0, argv_1);
          cache[(*cache_i)++] = (cache_item) { type, result };
          return result;
        }

        case MU_VECTOR_CORE:
        {
          const type_t *argv_0 = instantiate_single_type(induce, type->argv[0], scheme, target_scheme, cache, cache_i);
          const type_t *result = type;
          if (argv_0 != type->argv[0])
            result = vector_type(induce, argv_0);
          cache[(*cache_i)++] = (cache_item) { type, result };
          return result;
        }
      }
      break;

    case RECORD_TYPE:
    {
      type_t *mut;
      if ((mut = record_type_allocate(induce, type->argc)) == NULL)
        return NULL;

      _Bool is_same = 1;
      for (size_t i = 0; i < type->argc; i++) {
        mut->schema[i].name = type->schema[i].name;
        mut->schema[i].type = instantiate_single_type(induce, type->schema[i].type, scheme, target_scheme, cache, cache_i);
        if (mut->schema[i].type != type->schema[i].type)
          is_same = 0;
      }

      const type_t *result = type;
      if (!is_same)
        result = record_type_activate(mut);
      cache[(*cache_i)++] = (cache_item) { type, result };
      return result;
    }

    case VARIABLE_TYPE:
    {
      if (type->polymorphic_to != scheme)
        return type;

      const type_t *newvar;
      if ((newvar = variable_type(induce, target_scheme)) == NULL)
        return NULL;
      cache[(*cache_i)++] = (cache_item) { type, newvar };

      for (size_t i = 0; i < induce->sub_length; i++) {
        const induce_sub_t sub = induce->sub_data[i];
        if (sub.lower == type)
          append(induce, newvar, instantiate_single_type(induce, sub.upper, scheme, target_scheme, cache, cache_i));
        if (sub.upper == type)
          append(induce, instantiate_single_type(induce, sub.lower, scheme, target_scheme, cache, cache_i), newvar);
      }

      return newvar;
    }

    case SCHEME_TYPE:
      fprintf(stderr, "Unsupported higher rank polymorphism\n");
      abort();

    case JOIN_TYPE:
    {
      type_t *mut;
      if ((mut = join_type_allocate(induce, type->join_argc)) == NULL)
        return NULL;

      _Bool is_same = 1;
      for (size_t i = 0; i < type->argc; i++) {
        mut->join_argv[i] = instantiate_single_type(induce, type->join_argv[i], scheme, target_scheme, cache, cache_i);
        if (mut->join_argv[i] != type->join_argv[i])
          is_same = 0;
      }

      const type_t *result = type;
      if (!is_same)
        result = join_type_activate(mut);
      cache[(*cache_i)++] = (cache_item) { type, result };
      return result;
    }
  }
}

const type_t *instantiate_scheme(
    induce_t *induce, const type_t *type, open_scheme_t *target_scheme
) {
  assert(type->kind == SCHEME_TYPE);
  cache_item cache[100] = {0};
  size_t i = 0;
  return instantiate_single_type(induce, type->matter, type, target_scheme, cache, &i);
}

void mark_type(induce_t *induce, const type_t *type, _Bool negative, size_t rank) {
  switch (type->kind) {
    case SIMPLE_TYPE:
      switch (type->core->kind) {
        case MU_BOOLEAN_CORE: break;
        case MU_INTEGER_CORE: break;

        case MU_LAMBDA_CORE:
          mark_type(induce, type->argv[0], !negative, rank);
          mark_type(induce, type->argv[1], negative, rank);
          break;

        case MU_VECTOR_CORE:
          mark_type(induce, type->argv[0], negative, rank);
          break;
      }
      break;

    case RECORD_TYPE:
      for (size_t i = 0; i < type->argc; i++)
        mark_type(induce, type->schema[i].type, negative, rank);
      break;

    case VARIABLE_TYPE:
      if (type->rank < rank)
        return;

      if (!negative) {
        ((type_t *) type)->positively_reachable = 1;

        for (size_t i = 0; i < induce->sub_length; i++) {
          induce_sub_t sub = induce->sub_data[i];
          if (sub.upper != type)
            continue;
          mark_type(induce, sub.lower, negative, rank);
        }
      } else {
        ((type_t *) type)->negatively_reachable = 1;

        for (size_t i = 0; i < induce->sub_length; i++) {
          induce_sub_t sub = induce->sub_data[i];
          if (sub.lower != type)
            continue;
          mark_type(induce, sub.upper, negative, rank);
        }
      }
      break;

    case SCHEME_TYPE:
      abort();

    case JOIN_TYPE:
      if (negative)
        return;

      for (size_t i = 0; i < type->join_argc; i++)
        mark_type(induce, type->join_argv[i], negative, rank);
      break;
  }
}

open_scheme_t *open_scheme(open_scheme_t *parent, const mu_node_t *node) {
  open_scheme_t *result;
  if ((result = malloc(sizeof(open_scheme_t))) == NULL)
    return NULL;
  *result = (open_scheme_t) {
    .induce = parent->induce, .node = node, .parent = parent, .rank = parent->rank + 1,
  };
  return result;
}

const type_t *scheme_type(induce_t *induce, const type_t *matter, size_t polymorphic_length, const type_t *head) {
  size_t size;
  if (rare((size = struct_size(type_t, polymorphic, polymorphic_length)) == 0))
    return NULL;

  type_t *result;
  if ((result = malloc(size)) == NULL)
    return NULL;
  *result = (type_t) {
    .kind = SCHEME_TYPE, .matter = matter, .polymorphic_length = polymorphic_length
  };

  size_t i = 0;
  for (const type_t *type = head; type != NULL; type = type->next)
    result->polymorphic[i++] = type;

  return result;
}

/**
 * @brief Restrict type @a a to be a subtype of @a b in the @a induce engine
 *
 * On allocation failure, @c errno is set by the allocator. This function can't
 * fail otherwise. The behavior is undefined if:
 *
 * - @a induce, @a a, or @a b is @c NULL
 * - @a a or @a b isn't in the same zone as that of the @a induce engine
 *
 * @param induce the induce engine to restrict @a a and @a b within
 * @param a the type to restrict to a subtype of @a b
 * @param b the type to restrict to a supertype of @a a
 */
static induce_t *restrict_type(
    induce_t *induce, const type_t *a, const type_t *b)
  __attribute__((nonnull));

/// Induce the type of the abstract @a node with the @a induce engine
static const type_t *node_induce(const mu_node_t *node, induce_t *induce, open_scheme_t *scheme)
  __attribute__((nonnull));

induce_t *induce_initialize(
    induce_t *induce,
    mu_engine_t *engine,
    mu_status_t *status,
    const detect_t *detect) {
  assert(detect_result(detect)->engine == engine);

  size_t node_length = engine->node_number;

  const mu_type_t **node_to_type;
  if ((node_to_type = malloc(sizeof(const mu_type_t *[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; node_to_type[i++] = NULL);

  const type_t **node_to_type_actual;
  if ((node_to_type_actual = malloc(sizeof(const mu_type_t *[node_length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < node_length; node_to_type_actual[i++] = NULL);

  size_t sub_volume = 1;
  induce_sub_t *sub_data;
  if ((sub_data = malloc(sizeof(induce_sub_t[sub_volume]))) == NULL)
    return NULL;
  for (size_t i = 0; i < sub_volume; sub_data[i++] = (induce_sub_t) {0});

  mu_core_t *boolean_core;
  if ((boolean_core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *boolean_core = (mu_core_t) { .kind = MU_BOOLEAN_CORE };

  mu_core_t *integer_core;
  if ((integer_core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *integer_core = (mu_core_t) { .kind = MU_INTEGER_CORE };

  size_t size;

  mu_core_t *lambda_core;
  size = struct_size(mu_core_t, variance, 2);
  if ((lambda_core = malloc(size)) == NULL)
    return NULL;
  *lambda_core = (mu_core_t) { .kind = MU_LAMBDA_CORE, .argc = 2 };
  lambda_core->variance[0] = MU_CONTRAVARIANCE;
  lambda_core->variance[1] = MU_COVARIANCE;

  mu_core_t *vector_core;
  size = struct_size(mu_core_t, variance, 1);
  if ((vector_core = malloc(size)) == NULL)
    return NULL;
  *vector_core = (mu_core_t) { .kind = MU_VECTOR_CORE, .argc = 1 };
  vector_core->variance[0] = MU_COVARIANCE;

  size_t length = engine->node_number;
  const mu_node_t **define_stmt_map;
  if ((define_stmt_map = malloc(sizeof(const mu_node_t *[length]))) == NULL)
    return NULL;
  for (size_t i = 0; i < length; i++)
    define_stmt_map[i] = NULL;

  *induce = (induce_t) {
    .engine = engine,
    .status = status,
    .detect = detect_result(detect),
    .node_length = node_length,
    .node_to_type = node_to_type,
    .node_to_type_actual = node_to_type_actual,
    .sub_volume = sub_volume,
    .sub_data = sub_data,

    .boolean_core = boolean_core,
    .integer_core = integer_core,
    .lambda_core = lambda_core,
    .vector_core = vector_core,

    .define_stmt_map = define_stmt_map,
  };
  return induce;
}

const mu_type_t *induce_node(induce_t *induce, const mu_node_t *root) {
  assert(root->as_stator.id < induce->node_length);

  open_scheme_t root_scheme = { .induce = induce, .node = root };
  open_scheme_t *scheme = &root_scheme;

  const mu_node_t *node = root, *next;
  do {
    const detect_result_t *detect = induce->detect;
    while ((next = detect_at(detect, node, node_cursor(node)->i++)) != NULL) {
      node = node_continue(node, next);

      if (node->kind != MU_DEFINE_STMT_NODE)
        continue;

      scheme = open_scheme(scheme, node);
    }

    // Induce the type of the node
    const type_t *type;
    if ((type = node_induce(node, induce, scheme)) == NULL)
      return NULL;

    if (node->kind == MU_DEFINE_STMT_NODE) {
      open_scheme_t *parent = scheme->parent;
      free(scheme);
      scheme = parent;
    }

    induce->node_to_type_actual[node->as_stator.id] = type;
  } while ((node = node_return(node)) != NULL);

  return (const mu_type_t *) induce_reveal(induce, root);
}

static induce_t *restrict_type(
    induce_t *induce, const type_t *a, const type_t *b) {
  assert(a->kind != SCHEME_TYPE && b->kind != SCHEME_TYPE);

  if (a == b)
    return induce;

  // If we don't have to continue into a or b, then just return
  for (size_t i = 0; i < induce->sub_length; i++) {
    if (induce->sub_data[i].lower == a && induce->sub_data[i].upper == b)
      return induce;
  }

  if (a->kind == SIMPLE_TYPE && b->kind == SIMPLE_TYPE) {
    if (a->core != b->core) {
      fprintf(stderr, "Type mismatch\n");
      abort();
    }

    const mu_core_t *core = a->core;

    for (size_t i = 0; i < core->argc; i++) {
      const type_t *lower = a->argv[i], *upper = b->argv[i];

      mu_variance_t variance = core->variance[i];
      assert(variance != MU_INVARIANCE);
      if (variance == MU_CONTRAVARIANCE) {
        const type_t *t = lower; lower = upper; upper = t;
      }

      if (restrict_type(induce, lower, upper) == NULL)
        return NULL;
    }

    append(induce, a, b);
    return induce;
  }

  if (a->kind == RECORD_TYPE && b->kind == RECORD_TYPE) {
    for (size_t j = 0; j < b->argc; j++) {
      for (size_t i = 0; i < a->argc; i++) {
        if (a->schema[i].name == b->schema[j].name) {
          if (restrict_type(induce, a->schema[i].type, b->schema[j].type) == NULL)
            return NULL;
          goto next;
        }
      }

      fprintf(stderr, "Type mismatch\n");
      abort();

    next:;
    }

    append(induce, a, b);
    return induce;
  }

  if (a->kind != VARIABLE_TYPE && b->kind != VARIABLE_TYPE) {
    fprintf(stderr, "Type mismatch\n");
    abort();
  }

  if (a->kind == VARIABLE_TYPE) {
    for (size_t i = 0; i < induce->sub_length; i++) {
      if (induce->sub_data[i].upper != a)
        continue;
      if (restrict_type(induce, induce->sub_data[i].lower, b) == NULL)
        return NULL;
    }
  }

  if (b->kind == VARIABLE_TYPE) {
    for (size_t j = 0; j < induce->sub_length; j++) {
      if (induce->sub_data[j].lower != b)
        continue;
      if (restrict_type(induce, a, induce->sub_data[j].upper) == NULL)
        return NULL;
    }
  }

  append(induce, a, b);
  return induce;
}

static const type_t *append(
    induce_t *induce, const type_t *restrict a, const type_t *restrict b) {
  for (size_t i = 0; i < induce->sub_length; i++) {
    induce_sub_t sub = induce->sub_data[i];
    if (sub.lower == a && sub.upper == b)
      return a;
  }

  if (induce->sub_length >= induce->sub_volume) {
    size_t volume = induce->sub_volume;
    if (rare(__builtin_mul_overflow(volume, 2, &volume)))
      return errno = ENOMEM, NULL;

    size_t size;
    if (rare(__builtin_mul_overflow(volume, sizeof(induce_sub_t), &size)))
      return errno = ENOMEM, NULL;

    induce_sub_t *sub_data = induce->sub_data;
    if ((sub_data = realloc(sub_data, size)) == NULL)
      return NULL;
    for (size_t i = induce->sub_volume; i < volume; i++)
      sub_data[i] = (induce_sub_t) {0};

    induce->sub_volume = volume;
    induce->sub_data = sub_data;
  }

  induce->sub_data[induce->sub_length++] = (induce_sub_t) {
    .lower = a, .upper = b };
  return b;
}

// ---------------------------------- Expr -------------------------------- {{{1

__attribute__((nonnull)) static const type_t *access_expr_induce(
    const mu_access_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;

  const type_t *record_ty;
  const type_member_t argv[] = {
    { .name = expr->name, .type = result }
  };
  if ((record_ty = record_type(induce, 1, argv)) == NULL)
    return NULL;

  const type_t *matter_type = induce_reveal(induce, &expr->matter->as_node);
  if (restrict_type(induce, matter_type, record_ty) == NULL)
    return NULL;
  return result;
}

__attribute__((nonnull)) static const type_t *boolean_expr_induce(
    const mu_boolean_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  return boolean_type(induce);
}

__attribute__((nonnull)) static const type_t *coerce_expr_induce(
    const mu_coerce_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  assert(0);
}

__attribute__((nonnull)) static const type_t *integer_expr_induce(
    const mu_integer_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  return integer_type(induce);
}

__attribute__((nonnull)) static const type_t *invoke_expr_induce(
    const mu_invoke_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const type_t *lambda = induce_reveal(induce, &expr->lambda->as_node);
  const type_t *matter = induce_reveal(induce, &expr->matter->as_node);

  /* if (lambda->kind == SIMPLE_TYPE && lambda->core == induce->lambda_core) { */
  /*   if (restrict_type(induce, matter, lambda->argv[0]) == NULL) */
  /*     return NULL; */
  /*   return lambda->argv[1]; */
  /* } */

  const type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;

  const type_t *lambda_ty;
  if ((lambda_ty = lambda_type(induce, matter, result)) == NULL)
    return NULL;

  if (restrict_type(induce, lambda, lambda_ty) == NULL)
    return NULL;
  return result;
}

__attribute__((nonnull)) static const type_t *lambda_expr_induce(
    const mu_lambda_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const type_t *argument = induce_reveal(induce, &expr->argument->as_node);
  const type_t *output = induce_reveal(induce, &expr->matter->as_node);
  return lambda_type(induce, argument, output);
}

__attribute__((nonnull)) static const type_t *name_expr_induce(
    const mu_name_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &expr->as_node)) == NULL)
    return variable_type(induce, scheme);

  const type_t *type = induce_reveal(induce, target);
  if (type->kind != SCHEME_TYPE)
    return type;

  // Instantiate the polymorphic type
  return instantiate_scheme(induce, type, scheme);
}

__attribute__((nonnull)) static const type_t *record_expr_induce(
    const mu_record_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  type_t *allocation;
  if ((allocation = record_type_allocate(induce, expr->argc)) == NULL)
    return NULL;

  size_t i = 0, j = expr->argc;
  for (size_t k = 0; k < expr->argc; k++) {
    const mu_name_t *member_name = expr->argv[k].name;
    const mu_expr_t *member_expr = expr->argv[k].expr;

    type_member_t member = {
      .name = member_name, .type = induce_reveal(induce, &member_expr->as_node),
    };
    allocation->schema[member.name == NULL ? i++ : --j] = member;
  }
  assert(i == j);
  qsort(&allocation->argv[j], expr->argc - j, sizeof(mu_expr_member_t),
      type_member_cmp);
  // TODO: check for duplicates
  return record_type_activate(allocation);
}

__attribute__((nonnull)) static const type_t *sequence_expr_induce(
    const mu_sequence_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  return induce_reveal(induce, &expr->output->as_node);
}

__attribute__((nonnull)) static const type_t *vector_expr_induce(
    const mu_vector_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const type_t *matter_type;
  if ((matter_type = variable_type(induce, scheme)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const type_t *type = induce_reveal(induce, &expr->argv[i]->as_node);
    if (restrict_type(induce, type, matter_type) == NULL)
      return NULL;
  }

  return vector_type(induce, matter_type);
}

__attribute__((nonnull)) static const type_t *zero_expr_induce(
    const mu_zero_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  return variable_type(induce, scheme);
}

// ---------------------------------- Sign -------------------------------- {{{1

__attribute__((nonnull)) static const type_t *boolean_sign_induce(
    const mu_boolean_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  return boolean_type(induce);
}

__attribute__((nonnull)) static const type_t *integer_sign_induce(
    const mu_integer_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  return integer_type(induce);
}

__attribute__((nonnull)) static const type_t *name_sign_induce(
    const mu_name_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &sign->as_node)) != NULL)
    return induce_reveal(induce, target);

  const type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;
  return result;
}

__attribute__((nonnull)) static const type_t *record_sign_induce(
    const mu_record_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  assert(0);
}

__attribute__((nonnull)) static const type_t *variable_sign_induce(
    const mu_variable_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  assert(0);
}

__attribute__((nonnull)) static const type_t *vector_sign_induce(
    const mu_vector_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const type_t *matter = induce_reveal(induce, &sign->matter->as_node);

  const type_t *result;
  if ((result = vector_type(induce, matter)) == NULL)
    return NULL;
  return result;
}

// ---------------------------------- Stmt -------------------------------- {{{1

__attribute__((nonnull, pure)) static const type_t *define_stmt_induce(
    const mu_define_stmt_t *stmt, induce_t *induce, open_scheme_t *scheme) {
  assert(scheme->node == &stmt->as_node);

  const type_t *expr_type = induce_reveal(induce, &stmt->expr->as_node);
  mark_type(induce, expr_type, 0, scheme->rank);

  size_t polymorphic_length = 0;
  const type_t *polymorphic = NULL;

  const type_t *type = scheme->link;
  while (type != NULL) {
    assert(type->rank == scheme->rank);

    const type_t *next = type->next;

    /* Does a type have to be both positively reachable and negatively reachable
     * from the type of the defined expr to be polymorphic? */
    /* What is a polymorphic type polymorphic to? Just this type scheme? Or all
     * type schemes above this? Or all type scheme below this? */

    /*
     * Not sure if this is true, but here are some thoughts:
     *
     * A variable type must be constrained somehow to be polymorphically useful.
     * If we have:
     *   foo :: a
     * Then, while theoretically foo is polymorphic, it's not any more useful
     * than:
     *   foo :: ⊥
     *
     * Similarly, this function:
     *   bar :: a -> ()
     * While theoretically polymorphic, is no more useful than:
     *   bar :: ⊤ -> ()
     *
     * A variable can be constrained by either appearing both positively and
     * negatively, being constrained by bounds, or (in the future) being
     * constrained by kind. For now, just do this:
     */
    if (type->positively_reachable && type->negatively_reachable) {
      ((type_t *) type)->next = polymorphic;
      polymorphic = type;
      ((type_t *) type)->rank = 0;
      polymorphic_length++;
    } else {
      ((type_t *) type)->next = scheme->parent->link;
      scheme->parent->link = type;
      ((type_t *) type)->rank--;
    }

    type = next;
  }

  if (polymorphic_length == 0)
    return expr_type;

  const type_t *result;
  if ((result = scheme_type(induce, expr_type, polymorphic_length, polymorphic)) == NULL)
    return NULL;

  for (const type_t *type = polymorphic; type != NULL; type = type->next)
    ((type_t *) type)->polymorphic_to = result;

  return result;
}

__attribute__((nonnull)) static const type_t *type_stmt_induce(
    const mu_type_stmt_t *stmt, induce_t *induce, open_scheme_t *scheme) {
  assert(0);
}

// ---------------------------------- View -------------------------------- {{{1

__attribute__((nonnull)) static const type_t *variable_view_induce(
    const mu_variable_view_t *view, induce_t *induce, open_scheme_t *scheme) {
  const type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;
  return result;
}

// -------------------------------- Abstract ------------------------------ {{{1

static const type_t *node_induce(const mu_node_t *node, induce_t *induce, open_scheme_t *scheme) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_NODE: \
      return lower##_induce((const mu_##lower##_t *) node, induce, scheme);
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}
