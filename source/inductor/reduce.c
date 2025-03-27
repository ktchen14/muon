#include "../common.h"
#include "core.h"
#include "induce.h"

#include "../stator/node.h"
#include "coercion.h"
#include "type.h"
#include "universe.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define evince induce_reveal

const mu_coercion_t *reduce_coercion(
    induce_t *induce, const mu_coercion_t *coercion);

/// Load the coercion that is assigned to the <em>coercion</em>'s course. If that
/// coercion is the course @a coercion itself, then return @c NULL.
__attribute__((nonnull, pure))
const mu_coercion_t *mu_edge_coercion_reload(
    const universe_t *universe, const mu_edge_coercion_t *coercion) {
  const mu_type_t *source = coercion->source;
  const mu_type_t *target = coercion->target;

  course_t *course = course_search(universe, source, target);
  assert(course != NULL);

  const mu_coercion_t *result;
  if ((result = course->coercion) == &coercion->as_coercion)
    return NULL;
  return result;
}

const mu_solution_t *reduce_type_to_join(
    induce_t *induce, const mu_variable_type_t *variable_type) {
  if (variable_type->join != NULL)
    return &variable_type->join->as_solution;

  universe_t *universe = &induce->universe;
  universe_iterator_t iterator;

  // Reduce each variable type that's a source to this variable type
  iterator = universe_iterator(universe, &variable_type->as_type, 0);
  for (course_t *course; (course = universe_next(&iterator)) != NULL;) {
    if (course->indirect > 1)
      continue;

    // If the course's source isn't a variable type, then length++
    const mu_variable_type_t *next_variable;
    if ((next_variable = mu_type_cast(course->source, next_variable)) == NULL)
      continue;

    // Otherwise, reduce it. Then add its join length to length.
    if (reduce_type_to_join(induce, next_variable) == NULL)
      return NULL;
    assert(next_variable->join != NULL);
  }

  // Take each type that we haven't definitely eliminated as a direct source for
  // this variable type (i.e. each course with indirect = 2), and simplify.
  universe_iterator_t it, jt, kt;
  it = universe_iterator(universe, &variable_type->as_type, 0);
  for (course_t *a; (a = universe_next(&it)) != NULL;) {
    if (a->indirect > 1 || a->source->kind == MU_VARIABLE_TYPE)
      continue;

    const mu_type_t *source = a->source;

    jt = universe_iterator(universe, &variable_type->as_type, 0);
    for (course_t *b; (b = universe_next(&jt)) != NULL;) {
      if (a == b)
        continue;
      if (b->indirect > 1 || b->source->kind == MU_VARIABLE_TYPE)
        continue;

      const mu_type_t *next_source = b->source;

      const mu_coercion_t *coercion;
      if ((coercion = retrieve_coercion(induce, next_source, source)) == NULL)
        return NULL;
      if (coercion == NO_SUCH_COERCION)
        continue;

      const mu_coercion_t *tail;
      if ((tail = coerce_with(a)) == NULL)
        return NULL;

      const mu_indirect_coercion_t *result;
      if ((result = mu_indirect_coercion(coercion, tail)) == NULL)
        return NULL;
      course_assign(b, &result->as_coercion);

      kt = universe_iterator(universe, &variable_type->as_type, 1);
      for (course_t *c; (c = universe_next(&kt)) != NULL;) {
        if (c->target->kind != MU_VARIABLE_TYPE)
          continue;

        course_t *course = course_search(&induce->universe, next_source, c->target);
        assert(course != NULL);

        const mu_coercion_t *tail;
        if ((tail = coerce_with(course)) == NULL)
          return NULL;

        const mu_indirect_coercion_t *result;
        if ((result = mu_indirect_coercion(coercion, tail)) == NULL)
          return NULL;
        course_assign(course, &result->as_coercion);
      }
    }
  }

  // At this point, each directish course (indirect <= 1) is an actual type that
  // should be put into the join.

  // Determine the length of the join
  iterator = universe_iterator(universe, &variable_type->as_type, 0);
  size_t length = 0;
  for (course_t *course; (course = universe_next(&iterator)) != NULL;) {
    if (course->indirect > 1 || course->source->kind == MU_VARIABLE_TYPE)
      continue;
    length += 1;
  }

  // Allocate a join
  mu_join_t *join;
  if ((join = join_allocate(induce, length)) == NULL)
    return NULL;
  join->argc = 0;

  iterator = universe_iterator(universe, &variable_type->as_type, 0);
  for (course_t *course; (course = universe_next(&iterator)) != NULL;) {
    if (course->indirect > 1 || course->source->kind == MU_VARIABLE_TYPE)
      continue;

    // Handle a normal type
    const mu_join_coercion_t *coercion;
    if ((coercion = mu_join_coercion(variable_type, join->argc)) == NULL)
      return NULL;
    join->argv[join->argc++] = course->source;

    course_assign(course, &coercion->as_coercion);
  }

  iterator = universe_iterator(universe, &variable_type->as_type, 0);
  for (course_t *course; (course = universe_next(&iterator)) != NULL;) {
    if (course->indirect > 1 || course->source->kind != MU_VARIABLE_TYPE)
      continue;

    const mu_variable_type_t *next_variable;
    next_variable = mu_type_cast(course->source, next_variable);
    assert(next_variable != NULL);

    // Okay. We have another variable type. We have make an unjoin coercion into
    // a join coercion.
    const mu_join_t *source_join = next_variable->join;

    mu_unjoin_coercion_t *allocation;
    if ((allocation = unjoin_coercion_allocate(source_join->argc)) == NULL)
      return NULL;

    for (size_t i = 0; i < source_join->argc; i++) {
      const mu_type_t *source = source_join->argv[i];

      course_t *course = course_search(&induce->universe, source, &variable_type->as_type);
      assert(course != NULL);

      const mu_coercion_t *coercion;
      if ((coercion = reduce_coercion(induce, course->coercion)) == NULL)
        return NULL;
      course_assign(course, coercion);

      allocation->argv[i] = coercion;
    }

    const mu_unjoin_coercion_t *unjoin_coercion;
    if ((unjoin_coercion = unjoin_coercion_activate(allocation, course->target)) == NULL)
      return NULL;
    course_assign(course, &unjoin_coercion->as_coercion);
  }

  const mu_join_t *result;
  if ((result = join_activate(join)) == NULL)
    return NULL;
  ((mu_variable_type_t *) variable_type)->join = result;

  return &result->as_solution;
}

const mu_coercion_t *reduce_coercion(
    induce_t *induce, const mu_coercion_t *coercion) {
  switch ON_ABSTRACT_OBJECT(coercion) {
    case MU_ID_COERCION:
      return coercion;

    case IS_KIND_OF(edge_coercion): {
      const mu_coercion_t *next_coercion;
      if ((next_coercion = mu_edge_coercion_reload(&induce->universe, edge_coercion)) != NULL)
        return reduce_coercion(induce, next_coercion);

      const mu_type_t *source = edge_coercion->source;
      const mu_type_t *target = edge_coercion->target;

      assert(target->kind == MU_VARIABLE_TYPE || source->kind == MU_VARIABLE_TYPE);

      if (target->kind == MU_VARIABLE_TYPE) {
        const mu_variable_type_t *v = (const mu_variable_type_t *) target;
        if (reduce_type_to_join(induce, v) == NULL)
          return NULL;

        next_coercion = mu_edge_coercion_reload(&induce->universe, edge_coercion);
        assert(next_coercion != NULL);
        return reduce_coercion(induce, next_coercion);
      } else {
        const mu_variable_type_t *v = (const mu_variable_type_t *) source;
        if (reduce_type_to_join(induce, v) == NULL)
          return NULL;

        const mu_join_t *source_join = v->join;
        assert(source_join != NULL);

        mu_unjoin_coercion_t *allocation;
        if ((allocation = unjoin_coercion_allocate(source_join->argc)) == NULL)
          return NULL;

        for (size_t i = 0; i < source_join->argc; i++) {
          const course_t *course;
          course = course_search(&induce->universe, source_join->argv[i], target);
          assert(course != NULL);

          const mu_coercion_t *coercion = course_coercion(course);
          assert(coercion != NULL);

          allocation->argv[i] = reduce_coercion(induce, coercion);
        }

        const mu_unjoin_coercion_t *result;
        if ((result = unjoin_coercion_activate(allocation, target)) == NULL)
          return NULL;
        return &result->as_coercion;
      }

      __builtin_unreachable();
    }

    case IS_KIND_OF(indirect_coercion): {
      const mu_coercion_t *head = indirect_coercion->head;
      head = reduce_coercion(induce, head);

      const mu_coercion_t *tail = indirect_coercion->tail;
      tail = reduce_coercion(induce, tail);

      const mu_indirect_coercion_t *result;
      if ((result = mu_indirect_coercion(head, tail)) == NULL)
        return NULL;
      return &result->as_coercion;
    }

    case IS_KIND_OF(variance_coercion): {
      const mu_core_type_t *target = variance_coercion->target;

      mu_variance_coercion_t *allocation;
      if ((allocation = variance_coercion_allocate(target)) == NULL)
        return NULL;

      for (size_t i = 0; i < target->core->argc; i++) {
        const mu_coercion_t *argument = variance_coercion->argv[i];
        allocation->argv[i] = reduce_coercion(induce, argument);
      }

      const mu_variance_coercion_t *result;
      if ((result = variance_coercion_activate(allocation)) == NULL)
        return NULL;
      return &result->as_coercion;
    }

    case MU_RECORD_COERCION:
      abort();

    case MU_JOIN_COERCION:
      return coercion;

    case IS_KIND_OF(unjoin_coercion): {
      size_t argc = unjoin_coercion->argc;

      mu_unjoin_coercion_t *allocation;
      if ((allocation = unjoin_coercion_allocate(argc)) == NULL)
        return NULL;

      for (size_t i = 0; i < argc; i++)
        allocation->argv[i] = reduce_coercion(induce, unjoin_coercion->argv[i]);

      const mu_unjoin_coercion_t *result;
      if ((result = unjoin_coercion_activate(allocation, unjoin_coercion->target)) == NULL)
        return NULL;
      return &result->as_coercion;
    }
  }
  __builtin_unreachable();
}

const mu_coercion_t *reduce_coercion_external(
    induce_t *induce, const mu_coercion_t *coercion) {
  debug("Reducing coercion ");
  mu_coercion_debug(coercion);

  const mu_coercion_t *result;
  if ((result = reduce_coercion(induce, coercion)) == NULL)
    return NULL;

  debug(" to ");
  mu_coercion_debug(result);
  debug("\n");

  return result;
}

const mu_type_t *reduce_node(induce_t *induce, const mu_node_t *root) {
  assert(root->id < induce->node_length);

  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);

    const mu_type_t *source = evince(induce, node);
    assert(source != NULL);

    const mu_coercion_t *coercion;
    if ((coercion = induce->node_to_coercion[node->id]) == NULL)
      continue;

    const mu_coercion_t *result;
    if ((result = reduce_coercion_external(induce, coercion)) == NULL)
      return NULL;
    induce->node_to_coercion[node->id] = result;
  } while ((node = node_return(node)) != NULL);

  return evince(induce, root);
}
