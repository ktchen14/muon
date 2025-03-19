#include "universe.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

universe_t *universe_initialize(universe_t *universe) {
  universe_edge_t *data;
  size_t volume = 1;
  if ((data = malloc(sizeof(universe_edge_t[volume]))) == NULL)
    return NULL;
  *universe = (universe_t) { .volume = 1, .data = data };
  return universe;
}

const universe_edge_t *universe_search(
    const universe_t *universe, const mu_type_t *source, const mu_type_t *target) {
  for (size_t i = 0; i < universe->length; i++) {
    const universe_edge_t *edge = &universe->data[i];
    if (edge->source == source && edge->target == target)
      return edge;
  }
  return NULL;
}

const universe_edge_t *universe_append(
    universe_t *universe,
    const mu_type_t *restrict source,
    const mu_type_t *restrict target,
    const tactic_t *tactic) {
  if (universe->length >= universe->volume) {
    size_t volume = universe->volume;
    if (rare(__builtin_mul_overflow(volume, 2, &volume)))
      return errno = ENOMEM, NULL;

    size_t size;
    if (rare(__builtin_mul_overflow(volume, sizeof(universe_edge_t), &size)))
      return errno = ENOMEM, NULL;

    universe_edge_t *data;
    if ((data = realloc(universe->data, size)) == NULL)
      return NULL;
    universe->data = data;

    universe->volume = volume;
  }

  universe_edge_t *result = &universe->data[universe->length++];
  *result = (universe_edge_t) {
    .source = source, .target = target, .tactic = tactic,
  };
  return result;
}

void mark_type_from_anywhere(
    const universe_t *universe, const mu_type_t *type, _Bool negative,
    const mu_variable_type_t *origin,
    type_link_t *link) {
  switch ON_ABSTRACT_OBJECT(type) {
    case IS_KIND_OF(core_type): {
      const mu_core_t *core = core_type->core;

      for (size_t i = 0; i < core->argc; i++) {
        mu_variance_t variance = core->argv[i].variance;
        assert(variance != MU_INVARIANCE);
        negative = variance == MU_COVARIANCE ? negative : !negative;
        mark_type_from_anywhere(universe, core_type->argv[i], negative, NULL, link);
      }

      break;
    }

    case IS_KIND_OF(variable_type): {
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

          // If this origin is a subtype of the earlier origin, then use this
          // origin instead
          if (universe_search(universe, &origin->as_type, &earlier_origin->as_type))
            ((mu_variable_type_t *) variable_type)->positively_entered_from = origin;

          // If this origin is a supertype of the earlier origin, then keep
          // the earlier origin
          else if (universe_search(universe, &earlier_origin->as_type, &origin->as_type))
            ;

          // If we found no relationship, then mark the variable as multihomed
          // by marking it as its own "entered from"
          else
            ((mu_variable_type_t *) variable_type)->positively_entered_from = variable_type;
        }

        for (size_t i = 0; i < universe->length; i++) {
          induce_edge_t sub = universe->data[i];
          if (sub.target != &variable_type->as_type)
            continue;
          mark_type_from_anywhere(universe, sub.source, negative, variable_type, link);
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

          // If this origin is a supertype of the earlier origin, then use this
          // origin instead
          if (universe_search(universe, &earlier_origin->as_type, &origin->as_type))
            ((mu_variable_type_t *) variable_type)->negatively_entered_from = origin;

          // If this origin is a subtype of the earlier origin, then keep the
          // earlier origin
          else if (universe_search(universe, &origin->as_type, &earlier_origin->as_type))
            ;

          // If we found no relationship, then mark the variable as multihomed
          // by marking it as its own "entered from"
          else
            ((mu_variable_type_t *) variable_type)->negatively_entered_from = variable_type;
        }

        for (size_t i = 0; i < universe->length; i++) {
          induce_edge_t sub = universe->data[i];
          if (sub.source != &variable_type->as_type)
            continue;
          mark_type_from_anywhere(universe, sub.target, negative, variable_type, link);
        }
      }
      break;
    }

    case MU_SCHEME_TYPE: abort();
  }
}

void mark_type_from_anywhere_first(
    const universe_t *universe, const mu_type_t *root, type_link_t *link) {
  mark_type_from_anywhere(universe, root, 0, NULL, link);
}
