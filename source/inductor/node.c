#include "node.h"

#include "common.h"
#include "induce.h"

#include "../detector/detect.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

/// Return the source type of the @a node in the @a inductor
[[gnu::nonnull, gnu::pure, gnu::returns_nonnull]]
static inline MuonType *node_type(const Inductor *inductor, MuonNode *node) {
  MuonType *result = node_source_type(inductor, node);
  return assert(result != NULL), result;
}

/**
 * @brief Restrain the @a node to evaluate to a subtype of @a type
 *
 * The behavior is undefined if:
 * - @a node and @a inductor don't have the same @a engine
 * - @a type and @a inductor don't have the same @a engine
 * - @a node was created after @a inductor
 * - @a node has no induced type
 */
MuonType *node_restrain(Inductor *inductor, MuonNode *node, MuonType *type) {
  assert(node->engine == inductor->engine);
  assert(type->engine == inductor->engine);

  MuonType *source = node_type(inductor, node);
  if (type_restrain(inductor, source, type, node) == NULL)
    return NULL;
  return inductor->node[node->id].target = type;
}

/// @internal Called to continue into the @a node
static MuonNode *on_continue(Inductor *inductor, MuonNode *node, MuonNode *next)
  MUON_HINT_SUFFIX(nonnull);

/// @internal Called, on return from a node, to inductor the type of the @a node
static MuonType *on_return(Inductor *inductor, MuonNode *node)
  MUON_HINT_SUFFIX(nonnull);

void *induce_script(MuonInductor *inductor, MuonScript *script) {
  MuonNode *node = &script->as_node;
  do {
    MuonNode *next;
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL) {
      if ((next = on_continue(inductor, node, next)) == NULL)
        goto except;
      node = next;
    }

    // Induce the type of the node
    MuonType *type;
    if ((type = on_return(inductor, node)) == NULL)
      goto except;

    size_t i = node->tag - MUON_MINORANT_NODE;
    const size_t *offset = &inductor->node_offset[i];
    assert(*offset + node->id < offset[1]);

    inductor->node[*offset + node->id].source = type;
  } while ((node = node_return(node)) != NULL);

  for (size_t i = 0; i < inductor->rule_length; i++) {
    Rule *rule = &inductor->edge[i];
    if (rule->tag != INDIRECT_RULE)
      continue;
    *rule = inductor->edge[--inductor->rule_length];
    i--;
  }
  return inductor;

except:
  while ((node = node_return(node)) != NULL) {}
  return NULL;
}

MUON_HINT(nonnull) static MuonType *access_expr_return(
    Inductor *inductor, MuonAccessExpr *expr) {
  MuonEngine *engine = inductor->engine;

  MuonRecordCore *core;
  if ((core = muon_record_core(engine, 1, &expr->name)) == NULL)
    return NULL;

  MuonImplicitType *implicit_type;
  if ((implicit_type = muon_implicit_type(engine)) == NULL)
    return NULL;

  MuonType *type_argv[] = {&implicit_type->as_type};
  MuonCoreType *record_type;
  if ((record_type = muon_core_type(engine, &core->as_core, type_argv)) == NULL)
    return NULL;

  MuonType *argv[] = {&record_type->as_type, &implicit_type->as_type};
  MuonCoreType *result;
  if ((result = muon_lambda_type(engine, argv[0], argv[1])) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *boolean_expr_return(
    Inductor *inductor, MuonBooleanExpr *expr) {
  MuonCoreType *result;
  if ((result = muon_boolean_type(inductor->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *cast_expr_return(
    Inductor *inductor, MuonCastExpr *expr) {
  MuonType *sign_type = node_type(inductor, &expr->sign->as_node);
  if (node_restrain(inductor, &expr->matter->as_node, sign_type) == NULL)
    return NULL;
  return sign_type;
}

MUON_HINT(nonnull) static MuonType *integer_expr_return(
    Inductor *inductor, MuonIntegerExpr *expr) {
  MuonCoreType *result;
  if ((result = muon_integer_type(inductor->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *invoke_expr_return(
    Inductor *inductor, MuonInvokeExpr *expr) {
  MuonEngine *engine = inductor->engine;

  MuonType *argument_type = node_type(inductor, &expr->argument->as_node);

  MuonImplicitType *result;
  if ((result = muon_implicit_type(engine)) == NULL)
    return NULL;

  MuonType *argv[] = {argument_type, &result->as_type};
  MuonCoreType *lambda_type;
  if ((lambda_type = muon_lambda_type(engine, argv[0], argv[1])) == NULL)
    return NULL;

  MuonType *target = &lambda_type->as_type;
  if (node_restrain(inductor, &expr->operator->as_node, target) == NULL)
    return NULL;

  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *lambda_expr_return(
    Inductor *inductor, MuonLambdaExpr *expr) {
  MuonType *argument = node_type(inductor, &expr->argument->as_node);
  MuonType *matter = node_type(inductor, &expr->matter->as_node);

  MuonCoreType *result;
  if ((result = muon_lambda_type(inductor->engine, argument, matter)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *name_expr_return(
    Inductor *inductor, MuonNameExpr *expr) {
  MuonNode *target = detect_evince_loose(inductor->detect, &expr->as_node);
  if (target != NULL) {
    return node_type(inductor, target);
  }

  if (inductor->module != NULL) {
    for (size_t i = 0; i < inductor->module->argc; i++) {
      if (inductor->module->argv[i]->name == expr->name) {
        return inductor->module->argv[i]->type;
      }
    }
  }

  assert(target != NULL);
  return NULL;
}

MUON_HINT(nonnull) static MuonType *record_expr_return(
    Inductor *inductor, MuonRecordExpr *expr) {
  MuonEngine *engine = inductor->engine;

  struct MuonRecordCore *core_allocation;
  if ((core_allocation = record_core_allocate(engine, expr->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++)
    core_allocation->argv[i] = expr->argv[i]->name;
  /* qsort(&core_allocation->argv[j], expr->argc - j, sizeof(mu_expr_member_t),
   */
  /*     type_member_cmp); */
  // TODO: check for duplicates

  MuonRecordCore *core;
  if ((core = record_core_activate(core_allocation)) == NULL)
    return NULL;

  struct MuonCoreType *type_allocation;
  if ((type_allocation = core_type_allocate(engine, &core->as_core)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++)
    type_allocation->argv[i] = node_type(inductor, &expr->argv[i]->as_node);

  MuonCoreType *result;
  if (rare((result = core_type_activate(type_allocation)) == NULL))
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *switch_case_return(
    Inductor *inductor, MuonSwitchCase *node) {
  MuonNode *target;
  if ((target = detect_evince(inductor->detect, &node->as_node)) == NULL)
    abort();
  MuonType *case_type = node_type(inductor, target);
  assert(case_type->tag != MUON_SCHEME_TYPE);

  MuonType *expr_type = node_type(inductor, &node->expr->as_node);

  MuonCoreType *result;
  if ((result = muon_lambda_type(inductor->engine, case_type, expr_type))
      == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *switch_expr_return(
    Inductor *inductor, MuonSwitchExpr *expr) {
  MuonImplicitType *result;
  if ((result = muon_implicit_type(inductor->engine)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    MuonSwitchCase *argument = expr->argv[i];
    if (node_restrain(inductor, &argument->as_node, &result->as_type) == NULL)
      return NULL;
  }

  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *sequence_expr_return(
    Inductor *inductor, MuonSequenceExpr *expr) {
  MuonImplicitType *result;
  if ((result = muon_implicit_type(inductor->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *vector_expr_return(
    Inductor *inductor, MuonVectorExpr *expr) {
  MuonEngine *engine = inductor->engine;

  MuonImplicitType *matter_type;
  if ((matter_type = muon_implicit_type(engine)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    MuonNode *argument = &expr->argv[i]->as_node;
    if (node_restrain(inductor, argument, &matter_type->as_type) == NULL)
      return NULL;
  }

  MuonCoreType *result;
  if ((result = muon_vector_type(engine, &matter_type->as_type)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *expr_import_return(
    Inductor *inductor, MuonExprImport *import) {
  MuonImplicitType *result;
  if ((result = muon_implicit_type(inductor->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *boolean_sign_return(
    Inductor *inductor, MuonBooleanSign *sign) {
  MuonCoreType *result;
  if ((result = muon_boolean_type(inductor->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *integer_sign_return(
    Inductor *inductor, MuonIntegerSign *sign) {
  MuonCoreType *result;
  if ((result = muon_integer_type(inductor->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *lambda_sign_return(
    Inductor *inductor, MuonLambdaSign *sign) {
  MuonType *argument_type = node_type(inductor, &sign->argument->as_node);
  MuonType *output_type = node_type(inductor, &sign->output->as_node);

  MuonCoreType *result;
  if ((result = muon_lambda_type(inductor->engine, argument_type, output_type))
      == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *name_sign_return(
    Inductor *inductor, MuonNameSign *sign) {
  MuonNode *target = detect_evince(inductor->detect, &sign->as_node);
  assert(target != NULL);
  return node_type(inductor, target);
}

MUON_HINT(nonnull) static MuonType *record_sign_return(
    Inductor *inductor, MuonRecordSign *sign) {
  assert(0);
}

MUON_HINT(nonnull) static MuonType *vector_sign_return(
    Inductor *inductor, MuonVectorSign *sign) {
  MuonType *matter = node_type(inductor, &sign->matter->as_node);

  MuonCoreType *result;
  if ((result = muon_vector_type(inductor->engine, matter)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *variable_sign_return(
    Inductor *inductor, MuonVariableSign *sign) {
  MuonImplicitType *result;
  if ((result = muon_implicit_type(inductor->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *coercion_stmt_return(
    Inductor *inductor, MuonCoercionStmt *stmt) {
  MuonType *source = node_type(inductor, &stmt->source->as_node);
  MuonType *target = node_type(inductor, &stmt->target->as_node);

  MuonCoreType *lambda_type;
  if ((lambda_type = muon_lambda_type(inductor->engine, source, target))
      == NULL)
    return NULL;

  // TODO: ensure that this isn't tautological
  if (node_restrain(inductor, &stmt->expr->as_node, &lambda_type->as_type)
      == NULL)
    return NULL;

  MuonCoreType *source_core_type = muon_type_cast(source, source_core_type);
  assert(source_core_type != NULL);
  MuonCoreType *target_core_type = muon_type_cast(target, target_core_type);
  assert(target_core_type != NULL);

  // const mu_instance_t *instance;
  // if ((instance = mu_instance(
  //          source_core_type->core, target_core_type->core, stmt->expr))
  //     == NULL)
  //   return NULL;
  // inductor->instance[inductor->instance_length++] = instance;

  return &lambda_type->as_type;
}

MUON_HINT(nonnull) static MuonType *datatype_option_return(
    Inductor *inductor, MuonDatatypeOption *option) {
  MuonCore *core = inductor->datatype_core;
  assert(core != NULL);

  MuonCoreType *result;
  if ((result = muon_core_type(inductor->engine, core, NULL)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonNode *datatype_stmt_continue(
    Inductor *inductor, MuonNode *node, MuonDatatypeStmt *next) {
  MuonCustomCore *core;
  if ((core = mu_simple_core(inductor->engine, next->name)) == NULL)
    return NULL;
  inductor->datatype_core = &core->as_core;

  return node_continue(node, &next->as_node);
}

MUON_HINT(nonnull) static MuonType *datatype_stmt_return(
    Inductor *inductor, MuonDatatypeStmt *stmt) {
  // TODO: No arguments supported for now
  MuonCore *core = inductor->datatype_core;
  MuonCoreType *result;
  if ((result = muon_core_type(inductor->engine, core, NULL)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonNode *define_stmt_continue(
    Inductor *inductor, MuonNode *node, MuonDefineStmt *next) {
  if (muon_scheme_initiate(inductor->engine) == NULL)
    return NULL;
  return node_continue(node, &next->as_node);
}

MUON_HINT(nonnull) static MuonType *define_stmt_return(
    Inductor *inductor, MuonDefineStmt *stmt) {
  MuonType *matter = node_type(inductor, &stmt->expr->as_node);

  MuonSchemeType *result;
  if ((result = muon_scheme_type(inductor->engine, matter)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *record_view_return(
    Inductor *inductor, MuonRecordView *view) {
  MuonEngine *engine = inductor->engine;

  struct MuonRecordCore *core_allocation;
  if ((core_allocation = record_core_allocate(engine, view->argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < view->argc; i++)
    core_allocation->argv[i] = view->argv[i]->name;

  /* qsort(&core_allocation->argv[j], view->argc - j, sizeof(mu_view_member_t),
   */
  /*     type_member_cmp); */
  // TODO: check for duplicates

  MuonRecordCore *core;
  if (rare((core = record_core_activate(core_allocation)) == NULL))
    return NULL;

  struct MuonCoreType *type_allocation;
  if ((type_allocation = core_type_allocate(inductor->engine, &core->as_core))
      == NULL)
    return NULL;

  for (size_t i = 0; i < view->argc; i++)
    type_allocation->argv[i] = node_type(inductor, &view->argv[i]->as_node);

  MuonCoreType *result;
  if (rare((result = core_type_activate(type_allocation)) == NULL))
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *variable_view_return(
    Inductor *inductor, MuonVariableView *view) {
  MuonImplicitType *result;
  if ((result = muon_implicit_type(inductor->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

MUON_HINT(nonnull) static MuonType *expr_member_return(
    Inductor *inductor, MuonExprMember *member) {
  return node_type(inductor, &member->expr->as_node);
}

MUON_HINT(nonnull) static MuonType *view_member_return(
    Inductor *inductor, MuonViewMember *member) {
  return node_type(inductor, &member->view->as_node);
}

MUON_HINT(nonnull) static MuonType *script_return(
    Inductor *inductor, MuonScript *script) {
  // TODO: this is nonsense
  MuonCoreType *result;
  if ((result = muon_integer_type(inductor->engine)) == NULL)
    return NULL;
  return &result->as_type;
}

static MuonNode *on_continue(
    Inductor *inductor, MuonNode *node, MuonNode *next) {
  switch ON_ABSTRACT_NODE(next) {
    case IS_CONCRETE_NODE(MuonDatatypeStmt *datatype_stmt)
      return datatype_stmt_continue(inductor, node, datatype_stmt);

    case IS_CONCRETE_NODE(MuonDefineStmt *define_stmt)
      return define_stmt_continue(inductor, node, define_stmt);

    default:
      return node_continue(node, next);
  }
  __builtin_unreachable();
}

static MuonType *on_return(Inductor *inductor, MuonNode *node) {
  switch ON_ABSTRACT_TYPE(node) {
#define MUON_EMIT(Title, lower, UPPER) \
    case MUON_##UPPER##_NODE: \
      return lower##_return(inductor, (Muon##Title *) node);
    MUON_EACH_NODE_STEM(MUON_EMIT)
#undef MUON_EMIT
  }
  __builtin_unreachable();
}
