#include "expr.h"

#include "engine.h"
#include "name.h"
#include "node.h"

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

const mu_access_expr_t *mu_access_expr(
    mu_engine_t *engine, const mu_name_t *name, const mu_expr_t *matter) {
  assert(name->engine == engine);
  assert(matter->as_node.engine == engine);

  mu_access_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_access_expr_t))) == NULL)
    return NULL;
  *result = (mu_access_expr_t) {
    .as_expr.kind = MU_ACCESS_EXPR, .name = name, .matter = matter,
  };
  return assign_node(engine, result);
}

const mu_boolean_expr_t *mu_boolean_expr(mu_engine_t *engine, _Bool data) {
  mu_boolean_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_boolean_expr_t))) == NULL)
    return NULL;
  *result = (mu_boolean_expr_t) {
    .as_expr.kind = MU_BOOLEAN_EXPR, .data = data,
  };
  return assign_node(engine, result);
}

const mu_integer_expr_t *mu_integer_expr(mu_engine_t *engine, uint64_t data) {
  mu_integer_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_integer_expr_t))) == NULL)
    return NULL;
  *result = (mu_integer_expr_t) {
    .as_expr.kind = MU_INTEGER_EXPR, .data = data,
  };
  return assign_node(engine, result);
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
  return assign_node(engine, result);
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
  return assign_node(engine, result);
}

const mu_name_expr_t *mu_name_expr(mu_engine_t *engine, const mu_name_t *name) {
  assert(name->engine == engine);

  mu_name_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_name_expr_t))) == NULL)
    return NULL;
  *result = (mu_name_expr_t) {
    .as_expr.kind = MU_NAME_EXPR, .name = name,
  };
  return assign_node(engine, result);
}

const mu_native_expr_t *mu_native_expr(mu_engine_t *engine, const mu_name_t *name) {
  assert(name->engine == engine);

  mu_native_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_native_expr_t))) == NULL)
    return NULL;
  *result = (mu_native_expr_t) {
    .as_expr.kind = MU_NATIVE_EXPR, .name = name,
  };
  return assign_node(engine, result);
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
  return assign_node(engine, result);
}

const mu_record_expr_t *mu_record_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_member_t *argv[]) {
  assert(argc == 0 && argv == NULL || argc != 0 && argv != NULL);

  mu_record_expr_t *result;
  if ((result = record_expr_allocate(engine, argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];

  return record_expr_activate(result);
}

const mu_sequence_expr_t *mu_sequence_expr(
    mu_engine_t *engine, size_t argc, const mu_stmt_t *const argv[]) {
  assert(argc == 0 && argv == NULL || argc != 0 && argv != NULL);

  mu_sequence_expr_t *result;
  if ((result = sequence_expr_allocate(engine, argc)) == NULL)
    return NULL;

  for (size_t i = 0; i < argc; i++)
    result->argv[i] = argv[i];

  return sequence_expr_activate(result);
}

const mu_vector_expr_t *mu_vector_expr(
    mu_engine_t *engine, size_t argc, const mu_expr_t *const argv[]) {
  assert(argc == 0 && argv == NULL || argc != 0 && argv != NULL);

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

  return assign_node(engine, result);
}

const mu_zero_expr_t *mu_zero_expr(mu_engine_t *engine) {
  mu_zero_expr_t *result;
  if ((result = node_allocate(engine, sizeof(mu_zero_expr_t))) == NULL)
    return NULL;
  *result = (mu_zero_expr_t) { .as_expr.kind = MU_ZERO_EXPR };
  return assign_node(engine, result);
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
  return assign_node(engine, expr);
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
  return assign_node(engine, expr);
}

#include "debug.h"

void mu_access_expr_debug(const mu_access_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(name = ",
      DEBUG_KIND("AccessExpr"), DEBUG_ID(expr->as_node.id));
  mu_name_debug(expr->name);
  putc(')', stderr);
  debug_node_type(&expr->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() { expr_debug_with_coercion(expr->matter); }
}

void mu_boolean_expr_debug(const mu_boolean_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(data = %s)",
      DEBUG_KIND("BooleanExpr"),
      DEBUG_ID(expr->as_node.id),
      expr->data ? "true" : "false");
  debug_node_type(&expr->as_node);
  putc('\n', stderr);
}

void mu_integer_expr_debug(const mu_integer_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(data = %" PRIu64 ")",
    DEBUG_KIND("IntegerExpr"),
    DEBUG_ID(expr->as_node.id),
    expr->data);
  debug_node_type(&expr->as_node);
  putc('\n', stderr);
}

void mu_invoke_expr_debug(const mu_invoke_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID,
      DEBUG_KIND("InvokeExpr"), DEBUG_ID(expr->as_node.id));
  debug_node_type(&expr->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    expr_debug_with_coercion(expr->operator);
    expr_debug_with_coercion(expr->argument);
  }
}

void mu_lambda_expr_debug(const mu_lambda_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID,
      DEBUG_KIND("LambdaExpr"), DEBUG_ID(expr->as_node.id));
  debug_node_type(&expr->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    WITH_DEBUG_NEGATE() { mu_view_debug(expr->argument); }
    expr_debug_with_coercion(expr->matter);
  }
}

void mu_name_expr_debug(const mu_name_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(name = ",
      DEBUG_KIND("NameExpr"), DEBUG_ID(expr->as_node.id));
  mu_name_debug(expr->name);
  putc(')', stderr);
  debug_node_type(&expr->as_node);
  putc('\n', stderr);
}

void mu_native_expr_debug(const mu_native_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID "(name = ",
      DEBUG_KIND("NativeExpr"), DEBUG_ID(expr->as_node.id));
  mu_name_debug(expr->name);
  putc(')', stderr);
  debug_node_type(&expr->as_node);
  putc('\n', stderr);
}

void mu_expr_member_debug(const mu_expr_member_t *member) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID,
      DEBUG_KIND("ExprMember"), DEBUG_ID(member->as_node.id));

  if (member->name != NULL) {
    fprintf(stderr, "(name = ");
    mu_name_debug(member->name);
    putc(')', stderr);
  }

  debug_node_type(&member->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() { expr_debug_with_coercion(member->expr); }
}

void mu_record_expr_debug(const mu_record_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID,
      DEBUG_KIND("RecordExpr"), DEBUG_ID(expr->as_node.id));
  debug_node_type(&expr->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < expr->argc; i++)
      mu_expr_member_debug(expr->argv[i]);
  }
}

void mu_sequence_expr_debug(const mu_sequence_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID,
      DEBUG_KIND("SequenceExpr"), DEBUG_ID(expr->as_node.id));
  debug_node_type(&expr->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < expr->argc; i++)
      mu_stmt_debug(expr->argv[i]);
  }
}

void mu_vector_expr_debug(const mu_vector_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID,
      DEBUG_KIND("VectorExpr"), DEBUG_ID(expr->as_node.id));
  debug_node_type(&expr->as_node);
  putc('\n', stderr);

  WITH_DEBUG_INDENT() {
    for (size_t i = 0; i < expr->argc; i++)
      expr_debug_with_coercion(expr->argv[i]);
  }
}

void mu_zero_expr_debug(const mu_zero_expr_t *expr) {
  fprintf(stderr, "%*s", debug_indent, "");
  fprintf(stderr, PRIsKIND "#" PRIuID,
      DEBUG_KIND("ZeroExpr"), DEBUG_ID(expr->as_node.id));
  debug_node_type(&expr->as_node);
  putc('\n', stderr);
}
