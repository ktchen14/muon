#include "induce.h"
#include "universe.h"

#include "detect.h"
#include "../stator.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define evince induce_reveal

_Thread_local induce_t *debug_induce;

const mu_name_t *vector_access;
const mu_name_t *vector_join;

static const induce_edge_t *restrict_type_semiinternal(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b, _Bool direct);

const mu_variable_type_t *variable_type(induce_t *induce, open_scheme_t *scheme) {
  mu_variable_type_t *result;
  if ((result = malloc(sizeof(mu_variable_type_t))) == NULL)
    return NULL;

  *result = (mu_variable_type_t) {
    .as_type.kind = MU_VARIABLE_TYPE,
    .as_type.induce = induce,
    .as_type.id = induce->type_number++,
    .scheme_next = scheme->link,
    .rank = scheme->rank,
  };
  return scheme->link = result;
}

typedef struct {
  const mu_type_t *source;
  const mu_type_t *target;
} cache_item;

/* const type_t *instantiate_single_type( */
/*     induce_t *induce, */
/*     const type_t *type, */
/*     const type_t *scheme, */
/*     open_scheme_t *target_scheme, */
/*     cache_item *cache, */
/*     size_t *cache_i */
/* ) { */
/*   for (size_t i = 0; i < 100; i++) { */
/*     if (cache[i].source == type) */
/*       return cache[i].target; */
/*   } */

/*   switch (type->kind) { */
/*     case SIMPLE_TYPE: */
/*       switch (type->core->kind) { */
/*         case MU_BOOLEAN_CORE: */
/*         case MU_INTEGER_CORE: */
/*           cache[(*cache_i)++] = (cache_item) { type, type }; */
/*           return type; */

/*         case MU_LAMBDA_CORE: */
/*         { */
/*           const type_t *argv_0 = instantiate_single_type(induce, type->argv[0], scheme, target_scheme, cache, cache_i); */
/*           const type_t *argv_1 = instantiate_single_type(induce, type->argv[1], scheme, target_scheme, cache, cache_i); */

/*           const type_t *result = type; */
/*           if (argv_0 != type->argv[0] || argv_1 == type->argv[1]) */
/*             result = lambda_type(induce, argv_0, argv_1); */
/*           cache[(*cache_i)++] = (cache_item) { type, result }; */
/*           return result; */
/*         } */

/*         case MU_VECTOR_CORE: */
/*         { */
/*           const type_t *argv_0 = instantiate_single_type(induce, type->argv[0], scheme, target_scheme, cache, cache_i); */
/*           const type_t *result = type; */
/*           if (argv_0 != type->argv[0]) */
/*             result = vector_type(induce, argv_0); */
/*           cache[(*cache_i)++] = (cache_item) { type, result }; */
/*           return result; */
/*         } */
/*       } */
/*       break; */

/*     case RECORD_TYPE: */
/*     { */
/*       type_t *mut; */
/*       if ((mut = record_type_allocate(induce, type->argc)) == NULL) */
/*         return NULL; */

/*       _Bool is_same = 1; */
/*       for (size_t i = 0; i < type->argc; i++) { */
/*         mut->schema[i].name = type->schema[i].name; */
/*         mut->schema[i].type = instantiate_single_type(induce, type->schema[i].type, scheme, target_scheme, cache, cache_i); */
/*         if (mut->schema[i].type != type->schema[i].type) */
/*           is_same = 0; */
/*       } */

/*       const type_t *result = type; */
/*       if (!is_same) */
/*         result = record_type_activate(mut); */
/*       cache[(*cache_i)++] = (cache_item) { type, result }; */
/*       return result; */
/*     } */

/*     case VARIABLE_TYPE: */
/*     { */
/*       if (type->polymorphic_to != scheme) */
/*         return type; */

/*       const type_t *newvar; */
/*       if ((newvar = variable_type(induce, target_scheme)) == NULL) */
/*         return NULL; */
/*       cache[(*cache_i)++] = (cache_item) { type, newvar }; */

/*       for (size_t i = 0; i < induce->sub_length; i++) { */
/*         const induce_sub_t sub = induce->sub_data[i]; */
/*         if (sub.lower == type) */
/*           append(induce, newvar, instantiate_single_type(induce, sub.upper, scheme, target_scheme, cache, cache_i)); */
/*         if (sub.upper == type) */
/*           append(induce, instantiate_single_type(induce, sub.lower, scheme, target_scheme, cache, cache_i), newvar); */
/*       } */

/*       return newvar; */
/*     } */

/*     case SCHEME_TYPE: */
/*       fprintf(stderr, "Unsupported higher rank polymorphism\n"); */
/*       abort(); */

/*     case JOIN_TYPE: */
/*     { */
/*       type_t *mut; */
/*       if ((mut = join_type_allocate(induce, type->join_argc)) == NULL) */
/*         return NULL; */

/*       _Bool is_same = 1; */
/*       for (size_t i = 0; i < type->argc; i++) { */
/*         mut->join_argv[i] = instantiate_single_type(induce, type->join_argv[i], scheme, target_scheme, cache, cache_i); */
/*         if (mut->join_argv[i] != type->join_argv[i]) */
/*           is_same = 0; */
/*       } */

/*       const type_t *result = type; */
/*       if (!is_same) */
/*         result = join_type_activate(mut); */
/*       cache[(*cache_i)++] = (cache_item) { type, result }; */
/*       return result; */
/*     } */
/*   } */
/* } */

/* const type_t *instantiate_scheme( */
/*     induce_t *induce, const type_t *type, open_scheme_t *target_scheme */
/* ) { */
/*   assert(type->kind == SCHEME_TYPE); */
/*   cache_item cache[100] = {0}; */
/*   size_t i = 0; */
/*   return instantiate_single_type(induce, type->matter, type, target_scheme, cache, &i); */
/* } */

void mark_type(induce_t *induce, const mu_type_t *type, _Bool negative, size_t rank) {
  switch ON_ABSTRACT_OBJECT(type) {
    case IS_KIND_OF(core_type): {
      const mu_core_t *core = core_type->core;

      for (size_t i = 0; i < core_type->core->argc; i++) {
        const mu_type_t *next = core_type->argv[i];
        _Bool next_negative = negative;

        mu_variance_t variance = core->argv[i].variance;
        assert(variance != MU_INVARIANCE);
        if (variance == MU_CONTRAVARIANCE)
          next_negative = !next_negative;

        mark_type(induce, next, next_negative, rank);
      }

      break;
    }

    case IS_KIND_OF(variable_type):
      if (variable_type->rank < rank)
        return;

      if (!negative) {
        ((mu_variable_type_t *) variable_type)->positively_reachable = 1;

        for (size_t i = 0; i < induce->universe.length; i++) {
          induce_edge_t sub = induce->universe.data[i];
          if (sub.upper != &variable_type->as_type)
            continue;
          mark_type(induce, sub.lower, negative, rank);
        }
      } else {
        ((mu_variable_type_t *) variable_type)->negatively_reachable = 1;

        for (size_t i = 0; i < induce->universe.length; i++) {
          induce_edge_t sub = induce->universe.data[i];
          if (sub.lower != &variable_type->as_type)
            continue;
          mark_type(induce, sub.upper, negative, rank);
        }
      }
      break;

    case MU_SCHEME_TYPE:
      abort();
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

/**
 * @brief Restrict type @a a to be a subtype of @a b in the @a induce engine
 *
 * On allocation failure, @c errno is set by the allocator. This function can't
 * fail otherwise. The behavior is undefined if:
 *
 * - @a induce, @a a, or @a b is @c NULL
 *
 * This returns &SELF if @a a and @a b are identical.
 *
 * @param induce the induce engine to restrict @a a and @a b within
 * @param a the type to restrict to a subtype of @a b
 * @param b the type to restrict to a supertype of @a a
 */
static const induce_edge_t *restrict_type(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b)
  __attribute__((nonnull));

/// Induce the type of the abstract @a node with the @a induce engine
static const mu_type_t *node_induce(const mu_node_t *node, induce_t *induce, open_scheme_t *scheme)
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

  universe_t universe;
  if (rare(universe_initialize(&universe) == NULL))
    return NULL;

  mu_id_coercion_t *id_coercion;
  if ((id_coercion = malloc(sizeof(mu_id_coercion_t))) == NULL)
    return NULL;
  *id_coercion = (mu_id_coercion_t) { .as_coercion.kind = MU_ID_COERCION };

  mu_core_t *boolean_core;
  if ((boolean_core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *boolean_core = (mu_core_t) { .kind = MU_BOOLEAN_CORE, .induce = induce };

  mu_core_t *integer_core;
  if ((integer_core = malloc(sizeof(mu_core_t))) == NULL)
    return NULL;
  *integer_core = (mu_core_t) { .kind = MU_INTEGER_CORE, .induce = induce };

  size_t size;

  mu_core_t *lambda_core;
  size = struct_size(mu_core_t, argv, 2);
  if ((lambda_core = malloc(size)) == NULL)
    return NULL;
  *lambda_core = (mu_core_t) { .kind = MU_LAMBDA_CORE, .induce = induce, .argc = 2 };
  lambda_core->argv[0] = (mu_core_member_t) { .variance = MU_CONTRAVARIANCE };
  lambda_core->argv[1] = (mu_core_member_t) { .variance = MU_COVARIANCE };

  mu_core_t *vector_core;
  size = struct_size(mu_core_t, argv, 1);
  if ((vector_core = malloc(size)) == NULL)
    return NULL;
  *vector_core = (mu_core_t) { .kind = MU_VECTOR_CORE, .induce = induce, .argc = 1 };
  vector_core->argv[0] = (mu_core_member_t) { .variance = MU_COVARIANCE };

  *induce = (induce_t) {
    .engine = engine,
    .status = status,
    .detect = detect_result(detect),
    .node_length = node_length,
    .node_to_type = node_to_type,
    .universe = universe,

    .id_coercion = id_coercion,

    .boolean_core = boolean_core,
    .integer_core = integer_core,
    .lambda_core = lambda_core,
    .vector_core = vector_core,
  };
  return induce;
}

const mu_type_t *induce_node(induce_t *induce, const mu_node_t *root) {
  assert(root->id < induce->node_length);

  open_scheme_t root_scheme = { .induce = induce, .node = root };
  open_scheme_t *scheme = &root_scheme;

  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL) {
      node = node_continue(node, next);

      const mu_datatype_stmt_t *datatype_stmt;
      if ((datatype_stmt = mu_node_cast(node, datatype_stmt)) != NULL) {
        const mu_core_t *core;
        if ((core = mu_simple_core(induce, datatype_stmt->name)) == NULL)
          return NULL;
        induce->core[induce->core_length++] = core;
        induce->datatype_core = core;
      }

      if (node->kind != MU_DEFINE_STMT_NODE)
        continue;

      scheme = open_scheme(scheme, node);
    }

    // Induce the type of the node
    const mu_type_t *type;
    if ((type = node_induce(node, induce, scheme)) == NULL)
      return NULL;

    if (node->kind == MU_DEFINE_STMT_NODE) {
      open_scheme_t *parent = scheme->parent;
      free(scheme);
      scheme = parent;
    }

    induce->node_to_type[node->id] = type;
  } while ((node = node_return(node)) != NULL);

  return induce_reveal(induce, root);
}

static const tactic_t no_tactic = {0};

static const tactic_t *restrict_type_internal(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b, _Bool direct) {
  assert(a->kind != MU_SCHEME_TYPE && b->kind != MU_SCHEME_TYPE);

  if (a->kind == MU_CORE_TYPE && b->kind == MU_CORE_TYPE) {
    const mu_core_type_t *core_type_a = (const mu_core_type_t *) a;
    const mu_core_t *core_a = core_type_a->core;

    const mu_core_type_t *core_type_b = (const mu_core_type_t *) b;
    const mu_core_t *core_b = core_type_b->core;

    if (core_a->kind == MU_RECORD_CORE && core_b->kind == MU_RECORD_CORE) {
      const record_instance_t *instance;
      if (rare((instance = get_record_instance(induce, core_a, core_b)) == NULL))
        return NULL;

      for (size_t j = 0; j < core_b->argc; j++) {
        size_t i = instance->argv[j];
        if (restrict_type_semiinternal(induce, core_type_a->argv[i], core_type_b->argv[j], direct) == NULL)
          return NULL;
      }

      const record_tactic_t *result;
      if ((result = record_tactic_create(instance)) == NULL)
        return NULL;
      return &result->as_tactic;
    }

    if (core_type_a->core != core_type_b->core) {
      fprintf(stderr, "Type mismatch\n");
      abort();
    }

    const mu_core_t *core = core_type_a->core;

    for (size_t i = 0; i < core->argc; i++) {
      const mu_type_t *lower = core_type_a->argv[i], *upper = core_type_b->argv[i];

      mu_variance_t variance = core->argv[i].variance;
      assert(variance != MU_INVARIANCE);
      if (variance == MU_CONTRAVARIANCE) {
        const mu_type_t *t = lower; lower = upper; upper = t;
      }

      if (restrict_type_semiinternal(induce, lower, upper, direct) == NULL)
        return NULL;
    }

    const variance_tactic_t *result;
    if ((result = variance_tactic_create(core)) == NULL)
      return NULL;
    return &result->as_tactic;
  }

  if (a->kind != MU_VARIABLE_TYPE && b->kind != MU_VARIABLE_TYPE) {
    fprintf(stderr, "Type mismatch\n");
    abort();
  }

  if (a->kind == MU_VARIABLE_TYPE && b->kind == MU_VARIABLE_TYPE)
    direct = 0;

  const mu_variable_type_t *variable_a;
  if ((variable_a = mu_type_cast(a, variable_a)) != NULL) {
    for (size_t i = 0; i < induce->universe.length; i++) {
      if (induce->universe.data[i].upper != &variable_a->as_type)
        continue;
      if (restrict_type_semiinternal(induce, induce->universe.data[i].lower, b, direct) == NULL)
        return NULL;
    }
  }

  const mu_variable_type_t *variable_b;
  if ((variable_b = mu_type_cast(b, variable_b)) != NULL) {
    for (size_t j = 0; j < induce->universe.length; j++) {
      if (induce->universe.data[j].lower != &variable_b->as_type)
        continue;
      if (restrict_type_semiinternal(induce, a, induce->universe.data[j].upper, direct) == NULL)
        return NULL;
    }
  }

  return &no_tactic;
}

const induce_edge_t SELF = {0};

static const induce_edge_t *restrict_type_semiinternal(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b, _Bool direct) {
  if (a == b)
    return &SELF;

  // If we already have an edge a -> b then just return it
  const induce_edge_t *edge;
  if ((edge = universe_search(&induce->universe, a, b)) != NULL)
    return edge;

  const tactic_t *tactic;
  if ((tactic = restrict_type_internal(induce, a, b, direct)) == NULL)
    return NULL;
  if (tactic == &no_tactic)
    tactic = NULL;

  return universe_append(&induce->universe, a, b, direct, tactic);
}

static const induce_edge_t *restrict_type(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b) {
  return restrict_type_semiinternal(induce, a, b, 1);
}

// ---------------------------------- Expr -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *access_expr_induce(
    const mu_access_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;

  const mu_core_t *core;
  if ((core = single_record_core(induce, expr->name)) == NULL)
    return NULL;
  assert(core->kind == MU_RECORD_CORE);

  mu_core_type_t *allocation;
  if ((allocation = core_type_allocate(induce, core)) == NULL)
    return NULL;
  allocation->argv[0] = &result->as_type;

  const mu_core_type_t *record_type;
  if (rare((record_type = core_type_activate(allocation)) == NULL))
    return NULL;
  induce->aux[expr->as_node.id] = &record_type->as_type;

  const mu_type_t *matter_type = induce_reveal(induce, &expr->matter->as_node);
  if (restrict_type(induce, matter_type, &record_type->as_type) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *boolean_expr_induce(
    const mu_boolean_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_expr_induce(
    const mu_integer_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *invoke_expr_induce(
    const mu_invoke_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_type_t *operator_type = evince(induce, &expr->operator->as_node);
  const mu_type_t *argument_type = evince(induce, &expr->argument->as_node);

  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;

  const mu_core_type_t *lambda_type;
  if ((lambda_type = mu_lambda_type(induce, argument_type, &result->as_type)) == NULL)
    return NULL;
  induce->aux[expr->as_node.id] = &lambda_type->as_type;

  if (restrict_type(induce, operator_type, &lambda_type->as_type) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *lambda_expr_induce(
    const mu_lambda_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_type_t *argument_type = evince(induce, &expr->argument->as_node);
  const mu_type_t *output_type = evince(induce, &expr->matter->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *name_expr_induce(
    const mu_name_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &expr->as_node)) == NULL) {
    abort();
    const mu_variable_type_t *result;
    if ((result = variable_type(induce, scheme)) == NULL)
      return NULL;
    return &result->as_type;
  }

  const mu_type_t *result = induce_reveal(induce, target);
  if (result->kind != MU_SCHEME_TYPE)
    return result;

  // Instantiate the polymorphic type
  /* result = instantiate_scheme(induce, result, scheme); */
  return result;
}

__attribute__((nonnull)) static const mu_type_t *native_expr_induce(
    const mu_native_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_type_t *integer_type;
  if ((integer_type = mu_integer_type(induce)) == NULL)
    return NULL;

  const mu_core_type_t *vector_type;
  if ((vector_type = mu_vector_type(induce, &integer_type->as_type)) == NULL)
    return NULL;

  const mu_type_t *argument_type = &vector_type->as_type;
  const mu_type_t *output_type = &integer_type->as_type;
  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *record_expr_induce(
    const mu_record_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  mu_core_t *core_allocation;
  if ((core_allocation = record_core_allocate(induce, expr->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_name_t *name = expr->argv[i]->name;

    mu_core_member_t member = { .name = name };
    core_allocation->argv[i] = member;
  }
  /* qsort(&core_allocation->argv[j], expr->argc - j, sizeof(mu_expr_member_t), */
  /*     type_member_cmp); */
  // TODO: check for duplicates

  const mu_core_t *core;
  if (rare((core = record_core_activate(core_allocation)) == NULL))
    return NULL;

  mu_core_type_t *type_allocation;
  if ((type_allocation = core_type_allocate(induce, core)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++)
    type_allocation->argv[i] = induce_reveal(induce, &expr->argv[i]->as_node);

  const mu_core_type_t *result;
  if (rare((result = core_type_activate(type_allocation)) == NULL))
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *switch_case_induce(
    const mu_switch_case_t *node, induce_t *induce, open_scheme_t *scheme) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &node->as_node)) == NULL)
    abort();
  const mu_type_t *case_type = induce_reveal(induce, target);
  assert(case_type->kind != MU_SCHEME_TYPE);

  const mu_type_t *expr_type = evince(induce, &node->expr->as_node);

  const mu_core_type_t *result;
  if ((result = mu_lambda_type(induce, case_type, expr_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *switch_expr_induce(
    const mu_switch_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = induce_reveal(induce, &expr->argv[i]->as_node);
    if (restrict_type(induce, type, &result->as_type) == NULL)
      return NULL;
  }

  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *sequence_expr_induce(
    const mu_sequence_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *vector_expr_induce(
    const mu_vector_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *matter_type;
  if ((matter_type = variable_type(induce, scheme)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_type_t *type = induce_reveal(induce, &expr->argv[i]->as_node);
    if (restrict_type(induce, type, &matter_type->as_type) == NULL)
      return NULL;
  }

  const mu_core_type_t *result;
  if ((result = mu_vector_type(induce, &matter_type->as_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *zero_expr_induce(
    const mu_zero_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;
  return &result->as_type;
}

// ---------------------------------- Sign -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *boolean_sign_induce(
    const mu_boolean_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_sign_induce(
    const mu_integer_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *name_sign_induce(
    const mu_name_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const mu_node_t *target;
  if ((target = detect_evince(induce->detect, &sign->as_node)) != NULL)
    return induce_reveal(induce, target);

  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *record_sign_induce(
    const mu_record_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  assert(0);
}

__attribute__((nonnull)) static const mu_type_t *vector_sign_induce(
    const mu_vector_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const mu_type_t *matter = induce_reveal(induce, &sign->matter->as_node);

  const mu_core_type_t *result;
  if ((result = mu_vector_type(induce, matter)) == NULL)
    return NULL;
  return &result->as_type;
}

// ---------------------------------- Stmt -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *datatype_option_induce(
    const mu_datatype_option_t *option, induce_t *induce, open_scheme_t *scheme) {
  const mu_core_t *core = induce->datatype_core;
  assert(core != NULL);

  const mu_core_type_t *result;
  if ((result = mu_core_type(induce, core, NULL)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *datatype_stmt_induce(
    const mu_datatype_stmt_t *stmt, induce_t *induce, open_scheme_t *scheme) {
  // TODO: Fake this
  const mu_core_type_t *result;
  if ((result = mu_integer_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull, pure)) static const mu_type_t *define_stmt_induce(
    const mu_define_stmt_t *stmt, induce_t *induce, open_scheme_t *scheme) {
  assert(scheme->node == &stmt->as_node);

  const mu_type_t *expr_type = induce_reveal(induce, &stmt->expr->as_node);
  mark_type(induce, expr_type, 0, scheme->rank);

  size_t polymorphic_length = 0;
  mu_variable_type_t *polymorphic = NULL;

  mu_variable_type_t *type = scheme->link;
  while (type != NULL) {
    assert(type->rank == scheme->rank);

    mu_variable_type_t *next = type->scheme_next;

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
      type->scheme_next = polymorphic;
      polymorphic = type;
      type->rank = 0;
      polymorphic_length++;
    } else {
      type->scheme_next = scheme->parent->link;
      scheme->parent->link = type;
      type->rank--;
    }

    type = next;
  }

  if (polymorphic_length == 0)
    return expr_type;
  return expr_type;

  /* mu_scheme_type_t *allocation; */
  /* if ((allocation = scheme_type_allocate(induce, polymorphic_length)) == NULL) */
  /*   return NULL; */

  /* size_t i = 0; */
  /* for (mu_variable_type_t *type = polymorphic; type != NULL; type = type->scheme_next) { */
  /*   allocation->argv[i++] = type; */
  /*   type->polymorphic_to = allocation; */
  /* } */

  /* const mu_scheme_type_t *result; */
  /* if (rare((result = scheme_type_activate(allocation, expr_type)) == NULL)) */
  /*   return NULL; */
  /* return &result->as_type; */
}

// ---------------------------------- View -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *variable_view_induce(
    const mu_variable_view_t *view, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;
  return &result->as_type;
}

// -------------------------------- Abstract ------------------------------ {{{1

__attribute__((nonnull)) static const mu_type_t *expr_member_induce(
    const mu_expr_member_t *member, induce_t *induce, open_scheme_t *scheme) {
  return induce_reveal(induce, &member->expr->as_node);
}

static const mu_type_t *node_induce(const mu_node_t *node, induce_t *induce, open_scheme_t *scheme) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) \
    case MU_##upper##_NODE: \
      return lower##_induce((const mu_##lower##_t *) node, induce, scheme);
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  }
  __builtin_unreachable();
}
