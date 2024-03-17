#include "node.h"

#include "common.h"

#include <muon.h>
#include <ruby.h>
#include <stddef.h>
#include <stdint.h>

// Expands to VALUE cNode, etc.
#define EMIT(Title, lower, UPPER) VALUE c##Title;
EACH_ABSTRACT_NODE_STEM(EMIT)
#undef EMIT

enum {
  MUON_NODE = MUON_MINORANT_NODE + MUON_NODE_NUMBER,
  MUON_EXPR,
  MUON_SIGN,
  MUON_STMT,
  MUON_VIEW,
};

// Maps each node tag to its superclass and parent date type
static const struct {
  VALUE *klass; const rb_data_type_t *data_type; //-
} super[] = {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winitializer-overrides"

#define EMIT(T, l, UPPER) [MUON_##UPPER] = { &cNode, &node_type },
  EACH_ABSTRACT_NODE_STEM(EMIT)
#undef EMIT

  [MUON_NODE] = { &rb_cObject, NULL },

#define EMIT(Title, lower, UPPER, Super, super) \
    [MUON_##UPPER] = { &c##Super, &super##_type },
  MUON_EACH_EXPR_STEM(EMIT, Expr, expr)
  MUON_EACH_SIGN_STEM(EMIT, Sign, sign)
  MUON_EACH_STMT_STEM(EMIT, Stmt, stmt)
  MUON_EACH_VIEW_STEM(EMIT, View, view)
#undef EMIT

#pragma GCC diagnostic pop
};

static void node_mark(void *data) {
  MuonNode *node = data;

  size_t offset = offsetof(Engine, engine);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
  auto engine = (const Engine *) ((const char *) node->engine - offset);
#pragma GCC diagnostic pop
  rb_gc_mark(engine->native);
}

// Expands to const rb_data_type_t node_type { ... }, etc.
#define EMIT(Title, lower, UPPER) \
  const rb_data_type_t lower##_type = { \
    .wrap_struct_name = "Muon" #Title, \
    .function = { .dmark = node_mark }, \
    .parent = super[MUON_##UPPER].data_type, \
    .flags = RUBY_TYPED_FREE_IMMEDIATELY, \
  };
EACH_ABSTRACT_NODE_STEM(EMIT)
#undef EMIT

static VALUE initialize_variadic(VALUE self, int argc, const VALUE *argv) {
  return rb_obj_call_init(self, argc, argv), self;
}

#define initialize(self, ...) __extension__ ({ \
  VALUE _argv[] = { __VA_ARGS__ }; \
  initialize_variadic((self), (int) sizeof(_argv) / sizeof(VALUE), _argv); \
})

static VALUE node_initialize(int, VALUE *, VALUE self) {
  return self;
}

// AccessExpr.new(engine, name)
static VALUE access_expr_new(VALUE klass, VALUE rb_engine, VALUE rb_name) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = as_muon_name(engine, rb_name);
  MuonAccessExpr *node;
  if ((node = muon_access_expr(engine, name)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonAccessExpr");
  return initialize(as_access_expr(node), rb_engine, rb_name);
}

// BooleanExpr.new(engine, data)
static VALUE boolean_expr_new(VALUE klass, VALUE rb_engine, VALUE rb_data) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  _Bool data = RTEST(rb_data);
  MuonBooleanExpr *node;
  if ((node = muon_boolean_expr(engine, data)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonBooleanExpr");
  return initialize(as_boolean_expr(node), rb_engine, rb_data);
}

// CastExpr.new(engine, sign, matter)
static VALUE cast_expr_new(VALUE klass, VALUE rb_engine, VALUE rb_sign,
                           VALUE rb_matter) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonSign *sign = as_muon_sign(rb_sign);
  MuonExpr *matter = as_muon_expr(rb_matter);
  MuonCastExpr *node;
  if ((node = muon_cast_expr(engine, sign, matter)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonCastExpr");
  return initialize(as_cast_expr(node), rb_engine, rb_sign, rb_matter);
}

// IntegerExpr.new(engine, data)
static VALUE integer_expr_new(VALUE klass, VALUE rb_engine, VALUE rb_data) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  uint64_t data = NUM2ULL(rb_data);
  MuonIntegerExpr *node;
  if ((node = muon_integer_expr(engine, data)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonIntegerExpr");
  return initialize(as_integer_expr(node), rb_engine, rb_data);
}

// InvokeExpr.new(engine, operator, argument)
static VALUE invoke_expr_new(VALUE klass, VALUE rb_engine, VALUE rb_operator,
                             VALUE rb_argument) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonExpr *operator = as_muon_expr(rb_operator);
  MuonExpr *argument = as_muon_expr(rb_argument);
  MuonInvokeExpr *node;
  if ((node = muon_invoke_expr(engine, operator, argument)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonInvokeExpr");
  return initialize(as_invoke_expr(node), rb_engine, rb_operator, rb_argument);
}

// LambdaExpr.new(engine, argument, matter)
static VALUE lambda_expr_new(VALUE klass, VALUE rb_engine, VALUE rb_argument,
                             VALUE rb_matter) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonView *argument = as_muon_view(rb_argument);
  MuonExpr *matter = as_muon_expr(rb_matter);
  MuonLambdaExpr *node;
  if ((node = muon_lambda_expr(engine, argument, matter)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonLambdaExpr");
  return initialize(as_lambda_expr(node), rb_engine, rb_argument, rb_matter);
}

// NameExpr.new(engine, name)
static VALUE name_expr_new(VALUE klass, VALUE rb_engine, VALUE rb_name) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = as_muon_name(engine, rb_name);
  MuonNameExpr *node;
  if ((node = muon_name_expr(engine, name)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonNameExpr");
  return initialize(as_name_expr(node), rb_engine, rb_name);
}

// NativeExpr.new(engine, name)
static VALUE native_expr_new(VALUE klass, VALUE rb_engine, VALUE rb_name) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = as_muon_name(engine, rb_name);
  MuonNativeExpr *node;
  if ((node = muon_native_expr(engine, name)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonNativeExpr");
  return initialize(as_native_expr(node), rb_engine, rb_name);
}

// RecordExpr.new(engine, *argv)
static VALUE record_expr_new(int va_argc, VALUE *va_argv, VALUE klass) {
  VALUE rb_engine, rb_rest;
  rb_scan_args(va_argc, va_argv, "1*", &rb_engine, &rb_rest);

  MuonEngine *engine = as_muon_engine(rb_engine);
  size_t argc = (size_t) RARRAY_LEN(rb_rest);
  VALUE allocv;
  MuonExprMember **argv = RB_ALLOCV_N(MuonExprMember *, allocv, argc);
  for (size_t i = 0; i < argc; i++)
    argv[i] = as_muon_expr_member(RARRAY_AREF(rb_rest, i));

  MuonRecordExpr *node;
  if ((node = muon_record_expr(engine, argc, argv)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonRecordExpr");
  RB_ALLOCV_END(allocv);
  return initialize_variadic(as_record_expr(node), va_argc, va_argv);
}

// SequenceExpr.new(engine, *argv)
static VALUE sequence_expr_new(int va_argc, VALUE *va_argv, VALUE klass) {
  VALUE rb_engine, rb_rest;
  rb_scan_args(va_argc, va_argv, "1*", &rb_engine, &rb_rest);

  MuonEngine *engine = as_muon_engine(rb_engine);
  size_t argc = (size_t) RARRAY_LEN(rb_rest);
  VALUE allocv;
  MuonStmt **argv = RB_ALLOCV_N(MuonStmt *, allocv, argc);
  for (size_t i = 0; i < argc; i++)
    argv[i] = as_muon_stmt(RARRAY_AREF(rb_rest, i));

  MuonSequenceExpr *node;
  if ((node = muon_sequence_expr(engine, argc, argv)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonSequenceExpr");
  RB_ALLOCV_END(allocv);
  return initialize_variadic(as_sequence_expr(node), va_argc, va_argv);
}

// SwitchExpr.new(engine, *argv)
static VALUE switch_expr_new(int va_argc, VALUE *va_argv, VALUE klass) {
  VALUE rb_engine, rb_rest;
  rb_scan_args(va_argc, va_argv, "1*", &rb_engine, &rb_rest);

  MuonEngine *engine = as_muon_engine(rb_engine);
  size_t argc = (size_t) RARRAY_LEN(rb_rest);
  VALUE allocv;
  MuonSwitchCase **argv = RB_ALLOCV_N(MuonSwitchCase *, allocv, argc);
  for (size_t i = 0; i < argc; i++)
    argv[i] = as_muon_switch_case(RARRAY_AREF(rb_rest, i));

  MuonSwitchExpr *node;
  if ((node = muon_switch_expr(engine, argc, argv)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonSwitchExpr");
  RB_ALLOCV_END(allocv);
  return initialize_variadic(as_switch_expr(node), va_argc, va_argv);
}

// VectorExpr.new(engine, *argv)
static VALUE vector_expr_new(int va_argc, VALUE *va_argv, VALUE klass) {
  VALUE rb_engine, rb_rest;
  rb_scan_args(va_argc, va_argv, "1*", &rb_engine, &rb_rest);

  MuonEngine *engine = as_muon_engine(rb_engine);
  size_t argc = (size_t) RARRAY_LEN(rb_rest);
  VALUE allocv;
  MuonExpr **argv = RB_ALLOCV_N(MuonExpr *, allocv, argc);
  for (size_t i = 0; i < argc; i++)
    argv[i] = as_muon_expr(RARRAY_AREF(rb_rest, i));

  MuonVectorExpr *node;
  if ((node = muon_vector_expr(engine, argc, argv)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonVectorExpr");
  RB_ALLOCV_END(allocv);
  return initialize_variadic(as_vector_expr(node), va_argc, va_argv);
}

// BooleanSign.new(engine)
static VALUE boolean_sign_new(VALUE klass, VALUE rb_engine) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonBooleanSign *node;
  if ((node = muon_boolean_sign(engine)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonBooleanSign");
  return initialize(as_boolean_sign(node), rb_engine);
}

// IntegerSign.new(engine)
static VALUE integer_sign_new(VALUE klass, VALUE rb_engine) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonIntegerSign *node;
  if ((node = muon_integer_sign(engine)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonIntegerSign");
  return initialize(as_integer_sign(node), rb_engine);
}

// LambdaSign.new(engine, argument, output)
static VALUE lambda_sign_new(VALUE klass, VALUE rb_engine, VALUE rb_argument,
                             VALUE rb_output) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonSign *argument = as_muon_sign(rb_argument);
  MuonSign *output = as_muon_sign(rb_output);
  MuonLambdaSign *node;
  if ((node = muon_lambda_sign(engine, argument, output)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonLambdaSign");
  return initialize(as_lambda_sign(node), rb_engine, rb_argument, rb_output);
}

// NameSign.new(engine, name)
static VALUE name_sign_new(VALUE klass, VALUE rb_engine, VALUE rb_name) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = as_muon_name(engine, rb_name);
  MuonNameSign *node;
  if ((node = muon_name_sign(engine, name)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonNameSign");
  return initialize(as_name_sign(node), rb_engine, rb_name);
}

// RecordSign.new(engine, *pairs) where each pair is [name_or_nil, sign]
static VALUE record_sign_new(int va_argc, VALUE *va_argv, VALUE klass) {
  VALUE rb_engine, rb_rest;
  rb_scan_args(va_argc, va_argv, "1*", &rb_engine, &rb_rest);

  MuonEngine *engine = as_muon_engine(rb_engine);
  size_t argc = (size_t) RARRAY_LEN(rb_rest);
  VALUE allocv;
  MuonSignMember *argv = RB_ALLOCV_N(MuonSignMember, allocv, argc);
  for (size_t i = 0; i < argc; i++) {
    VALUE pair = rb_check_array_type(RARRAY_AREF(rb_rest, i));
    if (NIL_P(pair) || RARRAY_LEN(pair) != 2)
      rb_raise(rb_eArgError, "each member must be a [name, sign] pair");
    VALUE rb_name = rb_ary_entry(pair, 0);
    VALUE rb_sign = rb_ary_entry(pair, 1);
    MuonName *name = NIL_P(rb_name) ? NULL : as_muon_name(engine, rb_name);
    MuonSign *sign = as_muon_sign(rb_sign);
    argv[i] = (MuonSignMember){ .name = name, .sign = sign };
  }

  MuonRecordSign *node;
  if ((node = muon_record_sign(engine, argc, argv)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonRecordSign");
  RB_ALLOCV_END(allocv);
  return initialize_variadic(as_record_sign(node), va_argc, va_argv);
}

// VectorSign.new(engine, matter)
static VALUE vector_sign_new(VALUE klass, VALUE rb_engine, VALUE rb_matter) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonSign *matter = as_muon_sign(rb_matter);
  MuonVectorSign *node;
  if ((node = muon_vector_sign(engine, matter)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonVectorSign");
  return initialize(as_vector_sign(node), rb_engine, rb_matter);
}

// CoercionStmt.new(engine, source, target, expr)
static VALUE coercion_stmt_new(VALUE klass, VALUE rb_engine, VALUE rb_source,
                               VALUE rb_target, VALUE rb_expr) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonSign *source = as_muon_sign(rb_source);
  MuonSign *target = as_muon_sign(rb_target);
  MuonExpr *expr = as_muon_expr(rb_expr);
  MuonCoercionStmt *node;
  if ((node = muon_coercion_stmt(engine, source, target, expr)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonCoercionStmt");
  return initialize(as_coercion_stmt(node), rb_engine, rb_source, rb_target, rb_expr);
}

// DatatypeStmt.new(engine, name, *argv)
static VALUE datatype_stmt_new(int va_argc, VALUE *va_argv, VALUE klass) {
  VALUE rb_engine, rb_name, rb_rest;
  rb_scan_args(va_argc, va_argv, "2*", &rb_engine, &rb_name, &rb_rest);

  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = as_muon_name(engine, rb_name);
  size_t argc = (size_t) RARRAY_LEN(rb_rest);
  VALUE allocv;
  MuonDatatypeOption **argv = RB_ALLOCV_N(MuonDatatypeOption *, allocv, argc);
  for (size_t i = 0; i < argc; i++)
    argv[i] = as_muon_datatype_option(RARRAY_AREF(rb_rest, i));

  MuonDatatypeStmt *node;
  if ((node = muon_datatype_stmt(engine, name, argc, argv)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonDatatypeStmt");
  RB_ALLOCV_END(allocv);
  return initialize_variadic(as_datatype_stmt(node), va_argc, va_argv);
}

// DefineStmt.new(engine, name, expr)
static VALUE define_stmt_new(VALUE klass, VALUE rb_engine, VALUE rb_name,
                             VALUE rb_expr) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = as_muon_name(engine, rb_name);
  MuonExpr *expr = as_muon_expr(rb_expr);
  MuonDefineStmt *node;
  if ((node = muon_define_stmt(engine, name, expr)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonDefineStmt");
  return initialize(as_define_stmt(node), rb_engine, rb_name, rb_expr);
}

// RecordView.new(engine, *argv)
static VALUE record_view_new(int va_argc, VALUE *va_argv, VALUE klass) {
  VALUE rb_engine, rb_rest;
  rb_scan_args(va_argc, va_argv, "1*", &rb_engine, &rb_rest);

  MuonEngine *engine = as_muon_engine(rb_engine);
  size_t argc = (size_t) RARRAY_LEN(rb_rest);
  VALUE allocv;
  MuonViewMember **argv = RB_ALLOCV_N(MuonViewMember *, allocv, argc);
  for (size_t i = 0; i < argc; i++)
    argv[i] = as_muon_view_member(RARRAY_AREF(rb_rest, i));

  MuonRecordView *node;
  if ((node = muon_record_view(engine, argc, argv)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonRecordView");
  RB_ALLOCV_END(allocv);
  return initialize_variadic(as_record_view(node), va_argc, va_argv);
}

// VariableView.new(engine, name)
static VALUE variable_view_new(VALUE klass, VALUE rb_engine, VALUE rb_name) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = as_muon_name(engine, rb_name);
  MuonVariableView *node;
  if ((node = muon_variable_view(engine, name)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonVariableView");
  return initialize(as_variable_view(node), rb_engine, rb_name);
}

// ExprMember.new(engine, name, expr) — name may be nil
static VALUE expr_member_new(VALUE klass, VALUE rb_engine, VALUE rb_name,
                             VALUE rb_expr) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = NIL_P(rb_name) ? NULL : as_muon_name(engine, rb_name);
  MuonExpr *expr = as_muon_expr(rb_expr);
  MuonExprMember *node;
  if ((node = muon_expr_member(engine, name, expr)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonExprMember");
  return initialize(as_expr_member(node), rb_engine, rb_name, rb_expr);
}

// SwitchCase.new(engine, name, expr)
static VALUE switch_case_new(VALUE klass, VALUE rb_engine, VALUE rb_name,
                             VALUE rb_expr) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = as_muon_name(engine, rb_name);
  MuonExpr *expr = as_muon_expr(rb_expr);
  MuonSwitchCase *node;
  if ((node = muon_switch_case(engine, name, expr)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonSwitchCase");
  return initialize(as_switch_case(node), rb_engine, rb_name, rb_expr);
}

// DatatypeOption.new(engine, name)
static VALUE datatype_option_new(VALUE klass, VALUE rb_engine, VALUE rb_name) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = as_muon_name(engine, rb_name);
  MuonDatatypeOption *node;
  if ((node = muon_datatype_option(engine, name)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonDatatypeOption");
  return initialize(as_datatype_option(node), rb_engine, rb_name);
}

// ViewMember.new(engine, name, view)
static VALUE view_member_new(VALUE klass, VALUE rb_engine, VALUE rb_name,
                             VALUE rb_view) {
  MuonEngine *engine = as_muon_engine(rb_engine);
  MuonName *name = as_muon_name(engine, rb_name);
  MuonView *view = as_muon_view(rb_view);
  MuonViewMember *node;
  if ((node = muon_view_member(engine, name, view)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonViewMember");
  return initialize(as_view_member(node), rb_engine, rb_name, rb_view);
}

// Script.new(engine, *argv)
static VALUE script_new(int va_argc, VALUE *va_argv, VALUE klass) {
  VALUE rb_engine, rb_rest;
  rb_scan_args(va_argc, va_argv, "1*", &rb_engine, &rb_rest);

  MuonEngine *engine = as_muon_engine(rb_engine);
  size_t argc = (size_t) RARRAY_LEN(rb_rest);
  VALUE allocv;
  MuonStmt **argv = RB_ALLOCV_N(MuonStmt *, allocv, argc);
  for (size_t i = 0; i < argc; i++)
    argv[i] = as_muon_stmt(RARRAY_AREF(rb_rest, i));

  MuonScript *node;
  if ((node = muon_script(engine, argc, argv)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonScript");
  RB_ALLOCV_END(allocv);
  return initialize_variadic(as_script(node), va_argc, va_argv);
}

// -- Readers ---------------------------------------------------------------

static VALUE node_id(VALUE self) {
  return SIZET2NUM(as_muon_node(self)->id);
}

static VALUE access_expr_name(VALUE self) {
  return as_symbol(as_muon_access_expr(self)->name);
}

static VALUE boolean_expr_data(VALUE self) {
  return as_muon_boolean_expr(self)->data ? Qtrue : Qfalse;
}

static VALUE integer_expr_data(VALUE self) {
  return ULL2NUM(as_muon_integer_expr(self)->data);
}

// -- Expr readers --

static VALUE cast_expr_sign(VALUE self) {
  return as_sign(as_muon_cast_expr(self)->sign);
}

static VALUE cast_expr_matter(VALUE self) {
  return as_expr(as_muon_cast_expr(self)->matter);
}

static VALUE invoke_expr_operator(VALUE self) {
  return as_expr(as_muon_invoke_expr(self)->operator);
}

static VALUE invoke_expr_argument(VALUE self) {
  return as_expr(as_muon_invoke_expr(self)->argument);
}

static VALUE lambda_expr_argument(VALUE self) {
  return as_view(as_muon_lambda_expr(self)->argument);
}

static VALUE lambda_expr_matter(VALUE self) {
  return as_expr(as_muon_lambda_expr(self)->matter);
}

static VALUE name_expr_name(VALUE self) {
  return as_symbol(as_muon_name_expr(self)->name);
}

static VALUE native_expr_name(VALUE self) {
  return as_symbol(as_muon_native_expr(self)->name);
}

static VALUE record_expr_argv(VALUE self) {
  MuonRecordExpr *node = as_muon_record_expr(self);
  VALUE ary = rb_ary_new_capa((long) node->argc);
  for (size_t i = 0; i < node->argc; i++)
    rb_ary_push(ary, as_expr_member(node->argv[i]));
  return rb_ary_freeze(ary);
}

static VALUE sequence_expr_argv(VALUE self) {
  MuonSequenceExpr *node = as_muon_sequence_expr(self);
  VALUE ary = rb_ary_new_capa((long) node->argc);
  for (size_t i = 0; i < node->argc; i++)
    rb_ary_push(ary, as_stmt(node->argv[i]));
  return rb_ary_freeze(ary);
}

static VALUE switch_expr_argv(VALUE self) {
  MuonSwitchExpr *node = as_muon_switch_expr(self);
  VALUE ary = rb_ary_new_capa((long) node->argc);
  for (size_t i = 0; i < node->argc; i++)
    rb_ary_push(ary, as_switch_case(node->argv[i]));
  return rb_ary_freeze(ary);
}

static VALUE vector_expr_argv(VALUE self) {
  MuonVectorExpr *node = as_muon_vector_expr(self);
  VALUE ary = rb_ary_new_capa((long) node->argc);
  for (size_t i = 0; i < node->argc; i++)
    rb_ary_push(ary, as_expr(node->argv[i]));
  return rb_ary_freeze(ary);
}

// -- Sign readers --

static VALUE lambda_sign_argument(VALUE self) {
  return as_sign(as_muon_lambda_sign(self)->argument);
}

static VALUE lambda_sign_output(VALUE self) {
  return as_sign(as_muon_lambda_sign(self)->output);
}

static VALUE name_sign_name(VALUE self) {
  return as_symbol(as_muon_name_sign(self)->name);
}

static VALUE vector_sign_matter(VALUE self) {
  return as_sign(as_muon_vector_sign(self)->matter);
}

// -- Stmt readers --

static VALUE coercion_stmt_source(VALUE self) {
  return as_sign(as_muon_coercion_stmt(self)->source);
}

static VALUE coercion_stmt_target(VALUE self) {
  return as_sign(as_muon_coercion_stmt(self)->target);
}

static VALUE coercion_stmt_expr(VALUE self) {
  return as_expr(as_muon_coercion_stmt(self)->expr);
}

static VALUE datatype_stmt_name(VALUE self) {
  return as_symbol(as_muon_datatype_stmt(self)->name);
}

static VALUE datatype_stmt_argv(VALUE self) {
  MuonDatatypeStmt *node = as_muon_datatype_stmt(self);
  VALUE ary = rb_ary_new_capa((long) node->argc);
  for (size_t i = 0; i < node->argc; i++)
    rb_ary_push(ary, as_datatype_option(node->argv[i]));
  return rb_ary_freeze(ary);
}

static VALUE define_stmt_name(VALUE self) {
  return as_symbol(as_muon_define_stmt(self)->name);
}

static VALUE define_stmt_expr(VALUE self) {
  return as_expr(as_muon_define_stmt(self)->expr);
}

// -- View readers --

static VALUE record_view_argv(VALUE self) {
  MuonRecordView *node = as_muon_record_view(self);
  VALUE ary = rb_ary_new_capa((long) node->argc);
  for (size_t i = 0; i < node->argc; i++)
    rb_ary_push(ary, as_view_member(node->argv[i]));
  return rb_ary_freeze(ary);
}

static VALUE variable_view_name(VALUE self) {
  return as_symbol(as_muon_variable_view(self)->name);
}

// -- Auxiliary node readers --

static VALUE expr_member_name(VALUE self) {
  MuonName *name = as_muon_expr_member(self)->name;
  return name ? as_symbol(name) : Qnil;
}

static VALUE expr_member_expr(VALUE self) {
  return as_expr(as_muon_expr_member(self)->expr);
}

static VALUE switch_case_name(VALUE self) {
  return as_symbol(as_muon_switch_case(self)->name);
}

static VALUE switch_case_expr(VALUE self) {
  return as_expr(as_muon_switch_case(self)->expr);
}

static VALUE datatype_option_name(VALUE self) {
  return as_symbol(as_muon_datatype_option(self)->name);
}

static VALUE view_member_name(VALUE self) {
  MuonName *name = as_muon_view_member(self)->name;
  return name ? as_symbol(name) : Qnil;
}

static VALUE view_member_view(VALUE self) {
  return as_view(as_muon_view_member(self)->view);
}

static VALUE script_argv(VALUE self) {
  MuonScript *node = as_muon_script(self);
  VALUE ary = rb_ary_new_capa((long) node->argc);
  for (size_t i = 0; i < node->argc; i++)
    rb_ary_push(ary, as_stmt(node->argv[i]));
  return rb_ary_freeze(ary);
}

void Init_muon_node(VALUE mMuon) {
#define EMIT(Title, l, UPPER) \
    c##Title = rb_define_class_under(mMuon, #Title, *super[MUON_##UPPER].klass);
  EACH_ABSTRACT_NODE_STEM(EMIT)
#undef EMIT

  rb_define_method(cNode, "initialize", node_initialize, -1);
  rb_define_method(cNode, "id", node_id, 0);
  rb_undef_alloc_func(cNode);

  rb_undef_method(rb_singleton_class(cNode), "new");
  rb_undef_method(rb_singleton_class(cExpr), "new");
  rb_undef_method(rb_singleton_class(cSign), "new");
  rb_undef_method(rb_singleton_class(cStmt), "new");
  rb_undef_method(rb_singleton_class(cView), "new");

  rb_define_singleton_method(cAccessExpr, "new", access_expr_new, 2);
  rb_define_method(cAccessExpr, "name", access_expr_name, 0);
  rb_define_singleton_method(cBooleanExpr, "new", boolean_expr_new, 2);
  rb_define_method(cBooleanExpr, "data", boolean_expr_data, 0);
  rb_define_singleton_method(cCastExpr, "new", cast_expr_new, 3);
  rb_define_method(cCastExpr, "sign", cast_expr_sign, 0);
  rb_define_method(cCastExpr, "matter", cast_expr_matter, 0);
  rb_define_singleton_method(cIntegerExpr, "new", integer_expr_new, 2);
  rb_define_method(cIntegerExpr, "data", integer_expr_data, 0);
  rb_define_singleton_method(cInvokeExpr, "new", invoke_expr_new, 3);
  rb_define_method(cInvokeExpr, "operator", invoke_expr_operator, 0);
  rb_define_method(cInvokeExpr, "argument", invoke_expr_argument, 0);
  rb_define_singleton_method(cLambdaExpr, "new", lambda_expr_new, 3);
  rb_define_method(cLambdaExpr, "argument", lambda_expr_argument, 0);
  rb_define_method(cLambdaExpr, "matter", lambda_expr_matter, 0);
  rb_define_singleton_method(cNameExpr, "new", name_expr_new, 2);
  rb_define_method(cNameExpr, "name", name_expr_name, 0);
  rb_define_singleton_method(cNativeExpr, "new", native_expr_new, 2);
  rb_define_method(cNativeExpr, "name", native_expr_name, 0);
  rb_define_singleton_method(cRecordExpr, "new", record_expr_new, -1);
  rb_define_method(cRecordExpr, "argv", record_expr_argv, 0);
  rb_define_singleton_method(cSequenceExpr, "new", sequence_expr_new, -1);
  rb_define_method(cSequenceExpr, "argv", sequence_expr_argv, 0);
  rb_define_singleton_method(cSwitchExpr, "new", switch_expr_new, -1);
  rb_define_method(cSwitchExpr, "argv", switch_expr_argv, 0);
  rb_define_singleton_method(cVectorExpr, "new", vector_expr_new, -1);
  rb_define_method(cVectorExpr, "argv", vector_expr_argv, 0);

  rb_define_singleton_method(cBooleanSign, "new", boolean_sign_new, 1);
  rb_define_singleton_method(cIntegerSign, "new", integer_sign_new, 1);
  rb_define_singleton_method(cLambdaSign, "new", lambda_sign_new, 3);
  rb_define_method(cLambdaSign, "argument", lambda_sign_argument, 0);
  rb_define_method(cLambdaSign, "output", lambda_sign_output, 0);
  rb_define_singleton_method(cNameSign, "new", name_sign_new, 2);
  rb_define_method(cNameSign, "name", name_sign_name, 0);
  rb_define_singleton_method(cRecordSign, "new", record_sign_new, -1);
  rb_define_singleton_method(cVectorSign, "new", vector_sign_new, 2);
  rb_define_method(cVectorSign, "matter", vector_sign_matter, 0);

  rb_define_singleton_method(cCoercionStmt, "new", coercion_stmt_new, 4);
  rb_define_method(cCoercionStmt, "source", coercion_stmt_source, 0);
  rb_define_method(cCoercionStmt, "target", coercion_stmt_target, 0);
  rb_define_method(cCoercionStmt, "expr", coercion_stmt_expr, 0);
  rb_define_singleton_method(cDatatypeStmt, "new", datatype_stmt_new, -1);
  rb_define_method(cDatatypeStmt, "name", datatype_stmt_name, 0);
  rb_define_method(cDatatypeStmt, "argv", datatype_stmt_argv, 0);
  rb_define_singleton_method(cDefineStmt, "new", define_stmt_new, 3);
  rb_define_method(cDefineStmt, "name", define_stmt_name, 0);
  rb_define_method(cDefineStmt, "expr", define_stmt_expr, 0);

  rb_define_singleton_method(cRecordView, "new", record_view_new, -1);
  rb_define_method(cRecordView, "argv", record_view_argv, 0);
  rb_define_singleton_method(cVariableView, "new", variable_view_new, 2);
  rb_define_method(cVariableView, "name", variable_view_name, 0);

  rb_define_singleton_method(cExprMember, "new", expr_member_new, 3);
  rb_define_method(cExprMember, "name", expr_member_name, 0);
  rb_define_method(cExprMember, "expr", expr_member_expr, 0);
  rb_define_singleton_method(cSwitchCase, "new", switch_case_new, 3);
  rb_define_method(cSwitchCase, "name", switch_case_name, 0);
  rb_define_method(cSwitchCase, "expr", switch_case_expr, 0);
  rb_define_singleton_method(cDatatypeOption, "new", datatype_option_new, 2);
  rb_define_method(cDatatypeOption, "name", datatype_option_name, 0);
  rb_define_singleton_method(cViewMember, "new", view_member_new, 3);
  rb_define_method(cViewMember, "name", view_member_name, 0);
  rb_define_method(cViewMember, "view", view_member_view, 0);
  rb_define_singleton_method(cScript, "new", script_new, -1);
  rb_define_method(cScript, "argv", script_argv, 0);
}
