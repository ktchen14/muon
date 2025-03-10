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

#define evince induce_reveal

_Thread_local induce_t *debug_induce;

const induce_edge_t *search_edge(
    const induce_t *induce, const mu_type_t *a, const mu_type_t *b) {
  for (size_t i = 0; i < induce->edge_length; i++) {
    const induce_edge_t *edge = &induce->edge[i];
    if (edge->lower == a && edge->upper == b)
      return edge;
  }
  return NULL;
}

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

/// Register @a a <: @a b in the @a induce engine
const induce_edge_t *append_edge(
    induce_t *induce,
    const mu_type_t *restrict a,
    const mu_type_t *restrict b,
    coercion_t coercion) {
  if (induce->edge_length >= induce->edge_volume) {
    size_t volume = induce->edge_volume;
    if (rare(__builtin_mul_overflow(volume, 2, &volume)))
      return errno = ENOMEM, NULL;

    size_t size;
    if (rare(__builtin_mul_overflow(volume, sizeof(induce_edge_t), &size)))
      return errno = ENOMEM, NULL;

    induce_edge_t *sub_data = induce->edge;
    if ((sub_data = realloc(sub_data, size)) == NULL)
      return NULL;
    for (size_t i = induce->edge_volume; i < volume; i++)
      sub_data[i] = (induce_edge_t) {0};

    induce->edge_volume = volume;
    induce->edge = sub_data;
  }

  induce_edge_t edge = { .lower = a, .upper = b, .coercion = coercion };
  induce->edge[induce->edge_length] = edge;
  induce_edge_t *result = &induce->edge[induce->edge_length];
  induce->edge_length++;
  return result;
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
  switch (type->kind) {
    case MU_SIMPLE_TYPE: {
      const mu_simple_type_t *simple_type = (const mu_simple_type_t *) type;

      switch (simple_type->core->kind) {
        case MU_BOOLEAN_CORE: break;
        case MU_INTEGER_CORE: break;

        case MU_LAMBDA_CORE:
          mark_type(induce, simple_type->argv[0], !negative, rank);
          mark_type(induce, simple_type->argv[1], negative, rank);
          break;

        case MU_VECTOR_CORE:
          mark_type(induce, simple_type->argv[0], negative, rank);
          break;
      }
      break;
    }

    case MU_RECORD_TYPE: {
      const mu_record_type_t *record_type = (const mu_record_type_t *) type;

      for (size_t i = 0; i < record_type->argc; i++)
        mark_type(induce, record_type->argv[i].type, negative, rank);
      break;
    }

    case MU_VARIABLE_TYPE: {
      const mu_variable_type_t *variable_type = (const mu_variable_type_t *) type;

      if (variable_type->rank < rank)
        return;

      if (!negative) {
        ((mu_variable_type_t *) variable_type)->positively_reachable = 1;

        for (size_t i = 0; i < induce->edge_length; i++) {
          induce_edge_t sub = induce->edge[i];
          if (sub.upper != &variable_type->as_type)
            continue;
          mark_type(induce, sub.lower, negative, rank);
        }
      } else {
        ((mu_variable_type_t *) variable_type)->negatively_reachable = 1;

        for (size_t i = 0; i < induce->edge_length; i++) {
          induce_edge_t sub = induce->edge[i];
          if (sub.lower != &variable_type->as_type)
            continue;
          mark_type(induce, sub.upper, negative, rank);
        }
      }
      break;
    }

    case MU_SCHEME_TYPE:
      abort();

    case MU_JOIN_TYPE: {
      const mu_join_type_t *join_type = (const mu_join_type_t *) type;

      if (negative)
        return;

      for (size_t i = 0; i < join_type->argc; i++)
        mark_type(induce, join_type->argv[i], negative, rank);

      break;
    }
  }
}

void mark_type_from_anywhere(
    induce_t *induce,
    const mu_type_t *type,
    _Bool negative,
    const mu_variable_type_t *origin,
    type_link_t *link) {
  switch (type->kind) {
    case MU_SIMPLE_TYPE: {
      const mu_simple_type_t *simple_type = (const mu_simple_type_t *) type;

      switch (simple_type->core->kind) {
        case MU_BOOLEAN_CORE: break;
        case MU_INTEGER_CORE: break;

        case MU_LAMBDA_CORE:
          mark_type_from_anywhere(induce, simple_type->argv[0], !negative, NULL, link);
          mark_type_from_anywhere(induce, simple_type->argv[1], negative, NULL, link);
          break;

        case MU_VECTOR_CORE:
          mark_type_from_anywhere(induce, simple_type->argv[0], negative, NULL, link);
          break;
      }
      break;
    }

    case MU_RECORD_TYPE: {
      const mu_record_type_t *record_type = (const mu_record_type_t *) type;

      for (size_t i = 0; i < record_type->argc; i++)
        mark_type_from_anywhere(induce, record_type->argv[i].type, negative, NULL, link);
      break;
    }

    case MU_VARIABLE_TYPE: {
      const mu_variable_type_t *variable_type = (const mu_variable_type_t *) type;

      if (origin == NULL)
        origin = variable_type;

      // Add the variable to the link unless it's already there
      if (variable_type->debug_next == NULL) {
        ((mu_variable_type_t *) variable_type)->debug_next = link->next;
        link->next = variable_type;
      }

      if (!negative) {
        // If we've never entered type this, then just use the origin
        if (variable_type->positively_entered_from == NULL) {
          ((mu_variable_type_t *) variable_type)->positively_entered_from = origin;

        // We can't override an earlier origin of itself
        } else if (variable_type->positively_entered_from == variable_type) {

        // If we entered this type before, then we need to establish how that
        // earlier entrance relates to this origin
        } else {
          const mu_variable_type_t *earlier_origin = variable_type->positively_entered_from;

          for (size_t i = 0; i < induce->edge_length; i++) {
            induce_edge_t sub = induce->edge[i];

            // If this origin is a subtype of the earlier origin, then use this
            // origin instead
            if (sub.lower == &origin->as_type && sub.upper == &earlier_origin->as_type) {
              ((mu_variable_type_t *) variable_type)->positively_entered_from = origin;
              goto found_positive;

            // If this origin is a supertype of the earlier origin, then keep
            // the earlier origin
            } else if (sub.lower == &earlier_origin->as_type && sub.upper == &origin->as_type) {
              goto found_positive;
            }
          }

          // If we found no relationship, then mark the variable as multihomed
          // by marking it as its own "entered from"
          ((mu_variable_type_t *) variable_type)->positively_entered_from = variable_type;

          found_positive:;
        }

        for (size_t i = 0; i < induce->edge_length; i++) {
          induce_edge_t sub = induce->edge[i];
          if (sub.upper != &variable_type->as_type)
            continue;
          mark_type_from_anywhere(induce, sub.lower, negative, variable_type, link);
        }
      } else {
        // If we've never entered type this, then just use the origin
        if (variable_type->negatively_entered_from == NULL) {
          ((mu_variable_type_t *) variable_type)->negatively_entered_from = origin;

        // We can't override an earlier origin of itself
        } else if (variable_type->negatively_entered_from == variable_type) {

        // If we entered this type before, then we need to establish how that
        // earlier entrance relates to this origin
        } else {
          const mu_variable_type_t *earlier_origin = variable_type->negatively_entered_from;

          for (size_t i = 0; i < induce->edge_length; i++) {
            induce_edge_t sub = induce->edge[i];

            // If this origin is a supertype of the earlier origin, then use this
            // origin instead
            if (sub.lower == &earlier_origin->as_type && sub.upper == &origin->as_type) {
              ((mu_variable_type_t *) variable_type)->negatively_entered_from = origin;
              goto found_negative;

            // If this origin is a subtype of the earlier origin, then keep
            // the earlier origin
            } else if (sub.lower == &origin->as_type && sub.upper == &earlier_origin->as_type) {
              goto found_negative;
            }
          }

          // If we found no relationship, then mark the variable as multihomed
          // by marking it as its own "entered from"
          ((mu_variable_type_t *) variable_type)->negatively_entered_from = variable_type;

          found_negative:;
        }

        for (size_t i = 0; i < induce->edge_length; i++) {
          induce_edge_t sub = induce->edge[i];
          if (sub.lower != &variable_type->as_type)
            continue;
          mark_type_from_anywhere(induce, sub.upper, negative, variable_type, link);
        }
      }
      break;
    }

    case MU_SCHEME_TYPE:
      return;
      abort();

    case MU_JOIN_TYPE: {
      const mu_join_type_t *join_type = (const mu_join_type_t *) type;

      if (negative)
        return;

      // TODO: the join type itself should be the next origin. But we declared
      // origin as a variable type, so do an ugly cast.
      for (size_t i = 0; i < join_type->argc; i++)
        mark_type_from_anywhere(induce, join_type->argv[i], negative, (const mu_variable_type_t *) join_type, link);
      break;
    }
  }
}

void mark_type_from_anywhere_first(induce_t *induce, const mu_type_t *root, type_link_t *link) {
  mark_type_from_anywhere(induce, root, 0, NULL, link);
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

  size_t edge_volume = 1;
  induce_edge_t *edge;
  if ((edge = malloc(sizeof(induce_edge_t[edge_volume]))) == NULL)
    return NULL;
  for (size_t i = 0; i < edge_volume; edge[i++] = (induce_edge_t) {0});

  mu_id_coercion_t *id_coercion;
  if ((id_coercion = malloc(sizeof(mu_id_coercion_t))) == NULL)
    return NULL;
  *id_coercion = (mu_id_coercion_t) { .as_coercion.kind = MU_ID_COERCION };

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

  *induce = (induce_t) {
    .engine = engine,
    .status = status,
    .detect = detect_result(detect),
    .node_length = node_length,
    .node_to_type = node_to_type,
    .edge_volume = edge_volume,
    .edge = edge,

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

const mu_type_t *handle_node_reduction(induce_t *induce, const mu_node_t *root) {
  assert(root->id < induce->node_length);

  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    const mu_expr_t *expr;
    if ((expr = mu_node_cast(node, expr)) != NULL)
      expr_reduce(expr, induce);
  } while ((node = node_return(node)) != NULL);

  return induce_reveal(induce, root);
}

static coercion_t restrict_type_internal(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b) {
  assert(a->kind != MU_SCHEME_TYPE && b->kind != MU_SCHEME_TYPE);

  if (a->kind == MU_SIMPLE_TYPE && b->kind == MU_SIMPLE_TYPE) {
    const mu_simple_type_t *simple_a = (const mu_simple_type_t *) a;
    const mu_simple_type_t *simple_b = (const mu_simple_type_t *) b;

    if (simple_a->core != simple_b->core) {
      fprintf(stderr, "Type mismatch\n");
      abort();
    }

    const mu_core_t *core = simple_a->core;

    for (size_t i = 0; i < core->argc; i++) {
      const mu_type_t *lower = simple_a->argv[i], *upper = simple_b->argv[i];

      mu_variance_t variance = core->variance[i];
      assert(variance != MU_INVARIANCE);
      if (variance == MU_CONTRAVARIANCE) {
        const mu_type_t *t = lower; lower = upper; upper = t;
      }

      if (restrict_type(induce, lower, upper) == NULL)
        return NULL;
    }

    return "simple";
  }

  if (a->kind == MU_RECORD_TYPE && b->kind == MU_RECORD_TYPE) {
    const mu_record_type_t *record_a = (const mu_record_type_t *) a;
    const mu_record_type_t *record_b = (const mu_record_type_t *) b;

    for (size_t j = 0; j < record_b->argc; j++) {
      for (size_t i = 0; i < record_a->argc; i++) {
        if (record_a->argv[i].name == record_b->argv[j].name) {
          if (restrict_type(induce, record_a->argv[i].type, record_b->argv[j].type) == NULL)
            return NULL;
          goto next;
        }
      }

      fprintf(stderr, "Type mismatch\n");
      abort();

    next:;
    }

    return "record";
  }

  if (a->kind != MU_VARIABLE_TYPE && b->kind != MU_VARIABLE_TYPE) {
    fprintf(stderr, "Type mismatch\n");
    abort();
  }

  const mu_variable_type_t *variable_a;
  if ((variable_a = mu_type_cast(a, variable_a)) != NULL) {
    for (size_t i = 0; i < induce->edge_length; i++) {
      if (induce->edge[i].upper != &variable_a->as_type)
        continue;
      if (restrict_type(induce, induce->edge[i].lower, b) == NULL)
        return NULL;
    }
  }

  const mu_variable_type_t *variable_b;
  if ((variable_b = mu_type_cast(b, variable_b)) != NULL) {
    for (size_t j = 0; j < induce->edge_length; j++) {
      if (induce->edge[j].lower != &variable_b->as_type)
        continue;
      if (restrict_type(induce, a, induce->edge[j].upper) == NULL)
        return NULL;
    }
  }

  return "";
}

const induce_edge_t SELF = {0};

static const induce_edge_t *restrict_type(
    induce_t *induce, const mu_type_t *a, const mu_type_t *b) {
  if (a == b)
    return &SELF;

  // If we don't have to continue into a or b, then just return
  for (size_t i = 0; i < induce->edge_length; i++) {
    induce_edge_t *edge = &induce->edge[i];
    if (edge->lower == a && edge->upper == b)
      return edge;
  }

  coercion_t coercion;
  if ((coercion = restrict_type_internal(induce, a, b)) == NULL)
    return NULL;

  return append_edge(induce, a, b, coercion);
}

// ---------------------------------- Expr -------------------------------- {{{1

__attribute__((nonnull)) static const mu_type_t *access_expr_induce(
    const mu_access_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_variable_type_t *result;
  if ((result = variable_type(induce, scheme)) == NULL)
    return NULL;

  const mu_record_type_t *record_type;
  const mu_type_member_t argv[] = {
    { .name = expr->name, .type = &result->as_type }
  };
  if ((record_type = mu_record_type(induce, 1, argv)) == NULL)
    return NULL;
  induce->aux[expr->as_node.id] = &record_type->as_type;

  const mu_type_t *matter_type = induce_reveal(induce, &expr->matter->as_node);
  if (restrict_type(induce, matter_type, &record_type->as_type) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *boolean_expr_induce(
    const mu_boolean_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_simple_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_expr_induce(
    const mu_integer_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  const mu_simple_type_t *result;
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

  const mu_simple_type_t *lambda_type;
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

  const mu_simple_type_t *result;
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
  const mu_simple_type_t *integer_type;
  if ((integer_type = mu_integer_type(induce)) == NULL)
    return NULL;

  const mu_simple_type_t *vector_type;
  if ((vector_type = mu_vector_type(induce, &integer_type->as_type)) == NULL)
    return NULL;

  const mu_type_t *argument_type = &vector_type->as_type;
  const mu_type_t *output_type = &integer_type->as_type;
  const mu_simple_type_t *result;
  if ((result = mu_lambda_type(induce, argument_type, output_type)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *record_expr_induce(
    const mu_record_expr_t *expr, induce_t *induce, open_scheme_t *scheme) {
  mu_record_type_t *allocation;
  if ((allocation = record_type_allocate(induce, expr->argc)) == NULL)
    return NULL;

  size_t i = 0, j = expr->argc;
  for (size_t k = 0; k < expr->argc; k++) {
    const mu_name_t *member_name = expr->argv[k].name;
    const mu_expr_t *member_expr = expr->argv[k].expr;

    mu_type_member_t member = {
      .name = member_name, .type = induce_reveal(induce, &member_expr->as_node),
    };
    allocation->argv[member.name == NULL ? i++ : --j] = member;
  }
  assert(i == j);
  qsort(&allocation->argv[j], expr->argc - j, sizeof(mu_expr_member_t),
      type_member_cmp);
  // TODO: check for duplicates

  const mu_record_type_t *result;
  if (rare((result = record_type_activate(allocation)) == NULL))
    return NULL;
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

  const mu_simple_type_t *result;
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
  const mu_simple_type_t *result;
  if ((result = mu_boolean_type(induce)) == NULL)
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *integer_sign_induce(
    const mu_integer_sign_t *sign, induce_t *induce, open_scheme_t *scheme) {
  const mu_simple_type_t *result;
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

  const mu_simple_type_t *result;
  if ((result = mu_vector_type(induce, matter)) == NULL)
    return NULL;
  return &result->as_type;
}

// ---------------------------------- Stmt -------------------------------- {{{1

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

  mu_scheme_type_t *allocation;
  if ((allocation = scheme_type_allocate(induce, polymorphic_length)) == NULL)
    return NULL;

  size_t i = 0;
  for (mu_variable_type_t *type = polymorphic; type != NULL; type = type->scheme_next) {
    allocation->argv[i++] = type;
    type->polymorphic_to = allocation;
  }

  const mu_scheme_type_t *result;
  if (rare((result = scheme_type_activate(allocation, expr_type)) == NULL))
    return NULL;
  return &result->as_type;
}

__attribute__((nonnull)) static const mu_type_t *type_stmt_induce(
    const mu_type_stmt_t *stmt, induce_t *induce, open_scheme_t *scheme) {
  assert(0);
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
