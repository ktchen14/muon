#include <muon.h>
#include "analyzer.h"
#include "script.h"
#include "stator.h"
#include "status.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char buffer[4096];

#include "menu.h"
#include "node.h"

criteria_t *induce(const mu_script_t *script, const induce_menu_t *menu) {
  criteria_t *criteria = malloc(sizeof(criteria_t));
  criteria->length = 0;

  for (size_t i = 0; i < script->argc; i++) {
    const mu_node_t *node = &script->argv[i]->as_node;

    do {
      const mu_node_t *next;
      while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
        node = node_continue(node, next);

      criteria = node_induce(node, criteria, menu);
    } while ((node = node_return(node)) != NULL);
  }

  return criteria;
}

int main(int argc, char *argv[argc]) {
  const char *muon_name = argc > 0 ? argv[0] : "muon";
  if (argc < 2) {
    fprintf(stderr, "Usage: %s source\n", muon_name);
    return EXIT_FAILURE;
  }

  FILE *stream;
  if ((stream = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "%s: fopen(): %s\n", muon_name, strerror(errno));
    goto except_fopen;
  }

  size_t read;
  read = fread(buffer, 1, 4096, stream);
  buffer[read] = '\0';

  mu_engine_t engine = {0};
  mu_status_t status = {0};

  mu_script_t *script;

  if ((script = mu_read_script(&engine, &status, buffer)) == NULL) {
    fprintf(stderr, "%s: mu_read_script(): %s\n", muon_name, strerror(errno));
    goto except_read_script;
  }

  mu_script_debug(script);

  const mu_stmt_t *const *resolution = resolve_names(
      &engine, script);
  for (size_t i = 0; i < engine.stator_id; i++) {
    if (resolution[i] != NULL)
      fprintf(stderr, "Stator #%zu = Stator #%zu\n", i, resolution[i]->as_stator.id);
  }

  induce_menu_t induce_menu = {
    .engine = &engine,
    .node_to_stmt = resolution,
  };
  criteria_t *criteria = induce(script, &induce_menu);
  assert(criteria != NULL);

  typedef union {
    const mu_stator_t *stator;
    const mu_node_t *node;
    const mu_type_t *type;
  } node_or_type_t;
  node_or_type_t *node_to_node_or_type = malloc(
      sizeof(node_or_type_t[engine.stator_id])
  );
  for (size_t i = 0; i < engine.stator_id; i++)
    node_to_node_or_type[i] = (node_or_type_t) {0};

  const mu_type_t **type_to_type = malloc(sizeof(const mu_type_t *[engine.stator_id]));
  for (size_t i = 0; i < engine.stator_id; i++)
    type_to_type[i] = NULL;

  for (size_t i = 0; i < criteria->length; i++) {
    constraint_t *constraint = &criteria->data[i];

    const mu_stator_t *a, *b;
    memcpy(&a, &constraint->a, sizeof(const mu_stator_t *));
    memcpy(&b, &constraint->b, sizeof(const mu_stator_t *));

    while (stator_isnode(a)) {
      const mu_stator_t *stator = node_to_node_or_type[a->id].stator;
      if (stator == NULL)
        break;
      a = stator;
    }

    while (stator_istype(a)) {
      const mu_type_t *type = type_to_type[a->id];
      if (type == NULL)
        break;
      a = &type->as_stator;
    }

    while (stator_isnode(b)) {
      const mu_stator_t *stator = node_to_node_or_type[b->id].stator;
      if (stator == NULL)
        break;
      b = stator;
    }

    while (stator_istype(b)) {
      const mu_type_t *type = type_to_type[b->id];
      if (type == NULL)
        break;
      b = &type->as_stator;
    }

    if (stator_isnode(a) && stator_isnode(b)) {
      node_to_node_or_type[a->id].node = (const mu_node_t *) b;

    } else if (stator_isnode(a) && stator_istype(b)) {
      node_to_node_or_type[a->id].type = (const mu_type_t *) b;

    } else if (stator_istype(a) && stator_isnode(b)) {
      node_to_node_or_type[b->id].type = (const mu_type_t *) a;

    } else if (stator_istype(a) && stator_istype(b)) {
      const mu_type_t *type_a = (const mu_type_t *) a;
      const mu_type_t *type_b = (const mu_type_t *) b;

      if (type_a->kind == MU_VARIABLE_TYPE && type_b->kind == MU_VARIABLE_TYPE) {
        type_to_type[type_a->as_stator.id] = type_b;

      } else if (type_a->kind == MU_VARIABLE_TYPE && type_b->kind != MU_VARIABLE_TYPE) {
        type_to_type[type_a->as_stator.id] = type_b;

      } else if (type_a->kind != MU_VARIABLE_TYPE && type_b->kind == MU_VARIABLE_TYPE) {
        type_to_type[type_b->as_stator.id] = type_a;

      } else if (type_a->kind != MU_VARIABLE_TYPE && type_b->kind != MU_VARIABLE_TYPE) {
        fprintf(stderr, "Both type: %zu %zu\n", a->id, b->id);
        mu_type_debug(type_a);
        mu_type_debug(type_b);

      } else abort();
    } else abort();
  }

  for (size_t i = 0; i < script->argc; i++) {
    const mu_node_t *node = &script->argv[i]->as_node;

    do {
      const mu_node_t *next;
      while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
        node = node_continue(node, next);

      fprintf(stderr, "Resolving node %zu\n", node->as_stator.id);

      const mu_stator_t *stator = &node->as_stator;
      while (stator_isnode(stator)) {
        const mu_stator_t *next = node_to_node_or_type[stator->id].stator;
        if (next == NULL)
          break;
        stator = next;
      }

      while (stator_istype(stator)) {
        const mu_type_t *next = type_to_type[stator->id];
        if (next == NULL)
          break;
        stator = &next->as_stator;
      }

      if (stator_istype(stator)) {
        const mu_type_t *type = (const mu_type_t *) stator;
        fprintf(stderr, "Node %zu: ", node->as_stator.id);
        mu_type_debug(type);
      }
    } while ((node = node_return(node)) != NULL);
  }

  for (size_t i = 0; i < criteria->length; i++) {
    constraint_t *constraint = &criteria->data[i];
    fprintf(stderr, "Constraint: %zu = %zu\n",
        constraint->a.node->as_stator.id,
        constraint->b.node->as_stator.id);
  }

  return EXIT_SUCCESS;

except_read_script:
  fclose(stream);

except_fopen:
  return EXIT_FAILURE;
}
