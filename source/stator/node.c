#include "node.h"

#include "engine.h"
#include "name.h"

#include "../common.h"

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/// @internal Allocate a node of size @a size in the @a engine
__attribute__((malloc, nonnull))
static inline void *node_allocate(mu_engine_t *engine, size_t size) {
  if (rare((size = struct_size(node_header_t, data, size)) == 0))
    return errno = ENOMEM, NULL;

  node_header_t *header;
  if ((header = engine_allocate(engine, size)) == NULL)
    return NULL;
  *header = (node_header_t) {0};

  return header->data;
}

/// @internal Assign the abstract @a node to the @a engine
__attribute__((nonnull, returns_nonnull))
static inline mu_node_t *assign_node(mu_engine_t *engine, mu_node_t *node) {
  node->engine = engine;
  node->id = engine->node_number++;
  return node;
}

/* __attribute__((malloc, nonnull)) */
/* static inline const mu_node_t *node_create( */
/*     mu_engine_t *engine, size_t size, void *data) { */
/*   mu_node_t *result; */
/*   if ((result = node_allocate(engine, size)) == NULL) */
/*     return NULL; */
/*   memcpy(result, data, size); */
/*   return assign_node(engine, result); */
/* } */

const mu_access_expr_t *mu_access_expr(
    mu_engine_t *engine, const mu_name_t *name) {
  assert(name->engine == engine);

  mu_access_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_access_expr_t))) == NULL)
    return NULL;
  *result = (mu_access_expr_t) {
    .as_expr.kind = MU_ACCESS_EXPR, .name = name,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_boolean_expr_t *mu_boolean_expr(mu_engine_t *engine, _Bool data) {
  mu_boolean_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_boolean_expr_t))) == NULL)
    return NULL;
  *result = (mu_boolean_expr_t) {
    .as_expr.kind = MU_BOOLEAN_EXPR, .data = data,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_cast_expr_t *mu_cast_expr(
    mu_engine_t *engine, const mu_sign_t *sign, const mu_expr_t *matter) {
  mu_cast_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_cast_expr_t))) == NULL)
    return NULL;
  *result = (mu_cast_expr_t) {
    .as_expr.kind = MU_CAST_EXPR, .sign = sign, .matter = matter,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_integer_expr_t *mu_integer_expr(mu_engine_t *engine, uint64_t data) {
  mu_integer_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_integer_expr_t))) == NULL)
    return NULL;
  *result = (mu_integer_expr_t) {
    .as_expr.kind = MU_INTEGER_EXPR, .data = data,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_invoke_expr_t *mu_invoke_expr(
    mu_engine_t *engine, const mu_expr_t *operator, const mu_expr_t *argument) {
  assert(operator->as_node.engine == engine);
  assert(argument->as_node.engine == engine);

  mu_invoke_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_invoke_expr_t))) == NULL)
    return NULL;
  *result = (mu_invoke_expr_t) {
    .as_expr.kind = MU_INVOKE_EXPR, .operator = operator, .argument = argument,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_lambda_expr_t *mu_lambda_expr(
    mu_engine_t *engine, const mu_view_t *argument, const mu_expr_t *matter) {
  assert(argument->as_node.engine == engine);
  assert(matter->as_node.engine == engine);

  mu_lambda_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_lambda_expr_t))) == NULL)
    return NULL;
  *result = (mu_lambda_expr_t) {
    .as_expr.kind = MU_LAMBDA_EXPR, .argument = argument, .matter = matter,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_name_expr_t *mu_name_expr(mu_engine_t *engine, const mu_name_t *name) {
  assert(name->engine == engine);

  mu_name_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_name_expr_t))) == NULL)
    return NULL;
  *result = (mu_name_expr_t) { .as_expr.kind = MU_NAME_EXPR, .name = name };
  return assign_node(engine, &result->as_node), result;
}

const mu_native_expr_t *mu_native_expr(
    mu_engine_t *engine, const mu_name_t *name) {
  assert(name->engine == engine);

  mu_native_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_native_expr_t))) == NULL)
    return NULL;
  *result = (mu_native_expr_t) {
    .as_expr.kind = MU_NATIVE_EXPR, .name = name,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_expr_member_t *mu_expr_member(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr) {
  assert(name == NULL || name->engine == engine);
  assert(expr->as_node.engine == engine);

  mu_expr_member_t *result;
  if ((result = node_allocate(engine, sizeof(mu_expr_member_t))) == NULL)
    return NULL;
  *result = (mu_expr_member_t) {
    .as_node.kind = MU_EXPR_MEMBER_NODE, .name = name, .expr = expr,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_record_expr_t *mu_record_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_member_t *argv[]) {
  assert(argc == 0 || argv != NULL);

  mu_record_expr_t *result;
  if ((result = record_expr_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return record_expr_activate(result);
}

const mu_switch_case_t *mu_switch_case(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr) {
  assert(name->engine == engine);
  assert(expr->as_node.engine == engine);

  mu_switch_case_t *result;
  if ((result = node_allocate(engine, sizeof(mu_switch_case_t))) == NULL)
    return NULL;
  *result = (mu_switch_case_t) {
    .as_node.kind = MU_SWITCH_CASE_NODE, .name = name, .expr = expr,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_switch_expr_t *mu_switch_expr(
    mu_engine_t *engine, size_t argc, const mu_switch_case_t *const argv[argc]) {
  mu_switch_expr_t *result;
  if ((result = switch_expr_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return switch_expr_activate(result);
}

const mu_sequence_expr_t *mu_sequence_expr(
    mu_engine_t *engine, size_t argc, const mu_stmt_t *const argv[argc]) {
  mu_sequence_expr_t *result;
  if ((result = sequence_expr_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return sequence_expr_activate(result);
}

const mu_vector_expr_t *mu_vector_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *const argv[]) {
  assert(argc == 0 || argv != NULL);

  for (size_t i = 0; i < argc; i++) {
    assert(argv[i] != NULL);
    assert(argv[i]->as_node.engine == engine);
  }

  size_t size;
  if (rare((size = struct_size(mu_vector_expr_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_vector_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_vector_expr_t) {
    .as_expr.kind = MU_VECTOR_EXPR, .argc = argc,
  };

  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];

  return assign_node(engine, &result->as_node), result;
}

const mu_zero_expr_t *mu_zero_expr(mu_engine_t *engine) {
  mu_zero_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_zero_expr_t))) == NULL)
    return NULL;
  *result = (mu_zero_expr_t) { .as_expr.kind = MU_ZERO_EXPR };
  return assign_node(engine, &result->as_node), result;
}

mu_record_expr_t *record_expr_allocate(mu_engine_t *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_record_expr_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_record_expr_t) { .as_node.engine = engine, .argc = argc };
  return result;
}

const mu_record_expr_t *record_expr_activate(mu_record_expr_t *expr) {
  mu_engine_t *engine = (mu_engine_t *) expr->as_node.engine;

  for (size_t i = 0; i < expr->argc; i++) {
    assert(expr->argv[i] != NULL);
    assert(expr->argv[i]->as_node.engine == engine);
  }

  mu_record_expr_t source = {
    .as_expr.kind = MU_RECORD_EXPR, .argc = expr->argc
  };
  memcpy(expr, &source, offsetof(mu_record_expr_t, argv));
  return assign_node(engine, &expr->as_node), expr;
}

mu_switch_expr_t *switch_expr_allocate(mu_engine_t *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_switch_expr_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_switch_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_switch_expr_t) { .as_node.engine = engine, .argc = argc };
  return result;
}

const mu_switch_expr_t *switch_expr_activate(mu_switch_expr_t *expr) {
  mu_engine_t *engine = (mu_engine_t *) expr->as_node.engine;

  for (size_t i = 0; i < expr->argc; i++) {
    assert(expr->argv[i] != NULL);
    assert(expr->argv[i]->as_node.engine == engine);
  }

  mu_switch_expr_t source = {
    .as_expr.kind = MU_SWITCH_EXPR, .argc = expr->argc
  };
  memcpy(expr, &source, offsetof(mu_switch_expr_t, argv));
  return assign_node(engine, &expr->as_node), expr;
}

mu_sequence_expr_t *sequence_expr_allocate(mu_engine_t *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_sequence_expr_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_sequence_expr_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_sequence_expr_t) { .as_node.engine = engine, .argc = argc };
  return result;
}

const mu_sequence_expr_t *sequence_expr_activate(mu_sequence_expr_t *expr) {
  mu_engine_t *engine = (mu_engine_t *) expr->as_node.engine;

  for (size_t i = 0; i < expr->argc; i++) {
    assert(expr->argv[i] != NULL);
    assert(expr->argv[i]->as_node.engine == engine);
  }

  mu_sequence_expr_t source = {
    .as_expr.kind = MU_SEQUENCE_EXPR, .argc = expr->argc
  };
  memcpy(expr, &source, offsetof(mu_sequence_expr_t, argv));
  return assign_node(engine, &expr->as_node), expr;
}

const mu_boolean_sign_t *mu_boolean_sign(mu_engine_t *engine) {
  mu_boolean_sign_t *result;
  if ((result = node_allocate(engine, sizeof(mu_boolean_sign_t))) == NULL)
    return NULL;
  *result = (mu_boolean_sign_t) { .as_sign.kind = MU_BOOLEAN_SIGN };
  return assign_node(engine, &result->as_node), result;
}

const mu_integer_sign_t *mu_integer_sign(mu_engine_t *engine) {
  mu_integer_sign_t *result;
  if ((result = node_allocate(engine, sizeof(mu_integer_sign_t))) == NULL)
    return NULL;
  *result = (mu_integer_sign_t) { .as_sign.kind = MU_INTEGER_SIGN };
  return assign_node(engine, &result->as_node), result;
}

const mu_lambda_sign_t *mu_lambda_sign(
    mu_engine_t *engine, const mu_sign_t *argument, const mu_sign_t *output) {
  assert(argument->as_node.engine == engine);
  assert(output->as_node.engine == engine);

  mu_lambda_sign_t *result;
  if ((result = node_allocate(engine, sizeof(mu_lambda_sign_t))) == NULL)
    return NULL;
  *result = (mu_lambda_sign_t) {
    .as_sign.kind = MU_LAMBDA_SIGN, .argument = argument, .output = output,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_name_sign_t *mu_name_sign(mu_engine_t *engine, const mu_name_t *name) {
  assert(name->engine == engine);

  mu_name_sign_t *result;
  if ((result = node_allocate(engine, sizeof(mu_name_sign_t))) == NULL)
    return NULL;
  *result = (mu_name_sign_t) { .as_sign.kind = MU_NAME_SIGN, .name = name };
  return assign_node(engine, &result->as_node), result;
}

const mu_record_sign_t *mu_record_sign(
    mu_engine_t *engine, size_t argc, const mu_sign_member_t argv[]) {
  assert(argc == 0 || argv != NULL);

  for (size_t i = 0; i < argc; i++) {
    const mu_name_t *member_name = argv[i].name;
    const mu_sign_t *member_sign = argv[i].sign;
    assert(member_name == NULL || member_name->engine == engine);
    assert(member_sign != NULL);
    assert(member_sign->as_node.engine == engine);
  }

  size_t size;
  if (rare((size = struct_size(mu_record_sign_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_sign_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_record_sign_t) {
    .as_sign.kind = MU_RECORD_SIGN, .argc = argc,
  };

  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];

  return assign_node(engine, &result->as_node), result;
}

const mu_vector_sign_t *mu_vector_sign(
    mu_engine_t *engine, const mu_sign_t *matter) {
  assert(matter->as_node.engine == engine);

  mu_vector_sign_t *result;
  if ((result = node_allocate(engine, sizeof(mu_vector_sign_t))) == NULL)
    return NULL;
  *result = (mu_vector_sign_t) {
    .as_sign.kind = MU_VECTOR_SIGN, .matter = matter,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_coercion_stmt_t *mu_coercion_stmt(
    mu_engine_t *engine,
    const mu_sign_t *source,
    const mu_sign_t *target,
    const mu_expr_t *expr) {
  assert(source->as_node.engine == engine);
  assert(target->as_node.engine == engine);
  assert(expr->as_node.engine == engine);

  mu_coercion_stmt_t *result;
  if ((result = node_allocate(engine, sizeof(mu_coercion_stmt_t))) == NULL)
    return NULL;
  *result = (mu_coercion_stmt_t) {
    .as_stmt.kind = MU_COERCION_STMT,
    .source = source,
    .target = target,
    .expr = expr,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_datatype_option_t *mu_datatype_option(
    mu_engine_t *engine, const mu_name_t *name) {
  assert(name->engine == engine);

  mu_datatype_option_t *result;
  if ((result = node_allocate(engine, sizeof(mu_datatype_option_t))) == NULL)
    return NULL;
  *result = (mu_datatype_option_t) {
    .as_node.kind = MU_DATATYPE_OPTION_NODE, .name = name,
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_datatype_stmt_t *mu_datatype_stmt(
    mu_engine_t *engine,
    const mu_name_t *name,
    size_t argc,
    const mu_datatype_option_t *argv[/* argc */]) {
  mu_datatype_stmt_t *result;
  if ((result = datatype_stmt_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return datatype_stmt_activate(result, name);
}

const mu_define_stmt_t *mu_define_stmt(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *expr) {
  assert(name->engine == engine);
  assert(expr->as_node.engine == engine);

  mu_define_stmt_t *result;
  if ((result = node_allocate(engine, sizeof(mu_define_stmt_t))) == NULL)
    return NULL;
  *result = (mu_define_stmt_t) {
    .as_stmt.kind = MU_DEFINE_STMT, .name = name, .expr = expr,
  };
  return assign_node(engine, &result->as_node), result;
}

mu_datatype_stmt_t *datatype_stmt_allocate(mu_engine_t *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_datatype_stmt_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_datatype_stmt_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_datatype_stmt_t) { .as_node.engine = engine, .argc = argc };
  return result;
}

const mu_datatype_stmt_t *datatype_stmt_activate(
    mu_datatype_stmt_t *stmt, const mu_name_t *name) {
  mu_engine_t *engine = (mu_engine_t *) stmt->as_node.engine;

  assert(name->engine == engine);

  for (size_t i = 0; i < stmt->argc; i++) {
    assert(stmt->argv[i] != NULL);
    assert(stmt->argv[i]->as_node.engine == engine);
  }

  mu_datatype_stmt_t source = {
    .as_stmt.kind = MU_DATATYPE_STMT, .name = name, .argc = stmt->argc,
  };
  memcpy(stmt, &source, offsetof(mu_datatype_stmt_t, argv));
  return assign_node(engine, &stmt->as_node), stmt;
}

const mu_view_member_t *mu_view_member(
    mu_engine_t *engine, const mu_name_t *name, const mu_view_t *view) {
  assert(name == NULL || name->engine == engine);
  assert(view->as_node.engine == engine);

  mu_view_member_t *result;
  if ((result = node_allocate(engine, sizeof(mu_view_member_t))) == NULL)
    return NULL;
  *result = (mu_view_member_t) {
    .as_node.kind = MU_EXPR_MEMBER_NODE, .name = name, .view = view,
    .announce_length = node_announce_length(&view->as_node),
  };
  return assign_node(engine, &result->as_node), result;
}

const mu_record_view_t *mu_record_view(
    mu_engine_t *engine, size_t argc, const mu_view_member_t *argv[]) {
  assert(argc == 0 || argv != NULL);

  mu_record_view_t *result;
  if ((result = record_view_allocate(engine, argc)) == NULL)
    return NULL;
  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];
  return record_view_activate(result);
}

const mu_variable_view_t *mu_variable_view(
    mu_engine_t *engine, const mu_name_t *name) {
  mu_variable_view_t *result;
  if ((result = node_allocate(engine, sizeof(mu_variable_view_t))) == NULL)
    return NULL;
  *result = (mu_variable_view_t) {
    .as_view.kind = MU_VARIABLE_VIEW, .name = name,
  };
  return assign_node(engine, &result->as_node), result;
}

mu_record_view_t *record_view_allocate(mu_engine_t *engine, size_t argc) {
  size_t size;
  if (rare((size = struct_size(mu_record_view_t, argv, argc)) == 0))
    return errno = ENOMEM, NULL;

  mu_record_view_t *result;
  if ((result = node_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_record_view_t) { .as_node.engine = engine, .argc = argc };
  return result;
}

const mu_record_view_t *record_view_activate(mu_record_view_t *view) {
  mu_engine_t *engine = (mu_engine_t *) view->as_node.engine;

  for (size_t i = 0; i < view->argc; i++) {
    assert(view->argv[i] != NULL);
    assert(view->argv[i]->as_node.engine == engine);
  }

  size_t announce_length = 0;
  for (size_t i = 0; i < view->argc; i++) {
    size_t n = node_announce_length(&view->argv[i]->as_node);
    if (rare(__builtin_add_overflow(announce_length, n, &announce_length)))
      return NULL;
  }

  mu_record_view_t source = {
    .as_view.kind = MU_RECORD_VIEW, .argc = view->argc,
    .announce_length = announce_length,
  };
  memcpy(view, &source, offsetof(mu_record_view_t, argv));
  return assign_node(engine, &view->as_node), view;
}

#include "../inductor.h"

__attribute__((nonnull))
static inline void debug_node_type(const mu_node_t *node) {
  if (debug_induce == NULL)
    return;

  const mu_type_t *type;
  if ((type = evince_type(debug_induce, node)) == NULL)
    return;

  debug(" ∷ ");
  type_debug(type, 1);
}

__attribute__((nonnull))
static inline int debug_node_coercion(const mu_node_t *node) {
  if (debug_induce == NULL)
    return debug_indent;

  const mu_coercion_t *coercion;
  if ((coercion = debug_induce->node_to_coercion[node->id]) == NULL)
    return debug_indent;

  if (coercion->kind == MU_ID_COERCION)
    return debug_indent;

  debug("%*s", debug_indent, "");
  mu_coercion_debug(coercion);

  const mu_type_t *source_type;
  if ((source_type = evince_type(debug_induce, node)) != NULL) {
    const mu_type_t *target_type;
    if ((target_type = mu_coercion_target(coercion, source_type)) != NULL) {
      debug(" ∷ ");
      type_debug(target_type, 0);
    }
  }

  debug("\n");

  return debug_indent + 2;
}

/// Emit debugging information on the abstract @a node to the debug stream
void mu_node_debug(const mu_node_t *node) {
  // Kind -> Text, e.g. [MU_ACCESS_EXPR_NODE] = "AccessExpr"
  static const char *const KIND_TEXT[] = {
#define MU_EMIT(l, upper, title) [MU_##upper##_NODE] = #title,
    MU_EACH_NODE_KIND(MU_EMIT)
#undef MU_EMIT
  };
  const char *kind = KIND_TEXT[node->kind];

  int indent = debug_indent;
  debug_indent = debug_node_coercion(node);

  debug("%*s", debug_indent, "");
  debug(PRIsKIND, DEBUG_NODE_KIND(kind));

  switch ON_ABSTRACT_OBJECT(node) {
    case IS_KIND_OF(access_expr):
      debug("(name = " PRIsNAME ")", DEBUG_NAME(access_expr->name)); break;

    case IS_KIND_OF(boolean_expr):
      debug("(data = %s)", boolean_expr->data ? "true" : "false"); break;

    case IS_KIND_OF(integer_expr):
      debug("(data = %" PRIu64 ")", integer_expr->data); break;

    case IS_KIND_OF(name_expr):
      debug("(name = " PRIsNAME ")", DEBUG_NAME(name_expr->name)); break;

    case IS_KIND_OF(native_expr):
      debug("(name = " PRIsNAME ")", DEBUG_NAME(native_expr->name)); break;

    case IS_KIND_OF(expr_member):
      if (expr_member->name != NULL)
        debug("(name = " PRIsNAME ")", DEBUG_NAME(expr_member->name));
      break;

    case IS_KIND_OF(switch_case):
      debug("(name = " PRIsNAME ")", DEBUG_NAME(switch_case->name)); break;

    case IS_KIND_OF(name_sign):
      debug("(name = " PRIsNAME ")", DEBUG_NAME(name_sign->name)); break;

    case IS_KIND_OF(datatype_option):
      debug("(name = " PRIsNAME ")", DEBUG_NAME(datatype_option->name)); break;

    case IS_KIND_OF(datatype_stmt):
      debug("(name = " PRIsNAME ")", DEBUG_NAME(datatype_stmt->name)); break;

    case IS_KIND_OF(define_stmt):
      debug("(name = " PRIsNAME ")", DEBUG_NAME(define_stmt->name)); break;

    case IS_KIND_OF(view_member):
      if (view_member->name != NULL)
        debug("(name = " PRIsNAME ")", DEBUG_NAME(view_member->name));
      break;

    case IS_KIND_OF(variable_view):
      debug("(name = " PRIsNAME ")", DEBUG_NAME(variable_view->name)); break;

    default: break;
  }

  debug(" #%zu", node->id);

  debug_node_type(node);
  debug("\n");

  WITH_DEBUG_INDENT() {
    size_t i = 0;
    for (const mu_node_t *next; (next = node_at(node, i)) != NULL; i++)
      mu_node_debug(next);
  }

  debug_indent = indent;
}
