#ifndef MUON_RUBY_NODE_H
#define MUON_RUBY_NODE_H

#include <muon.h>
#include <ruby.h>

#define EACH_ABSTRACT_NODE_STEM(emit, ...) \
  emit(Node, node, NODE) \
  emit(Expr, expr, EXPR __VA_OPT__(,) __VA_ARGS__) \
  emit(Sign, sign, SIGN __VA_OPT__(,) __VA_ARGS__) \
  emit(Stmt, stmt, STMT __VA_OPT__(,) __VA_ARGS__) \
  emit(View, view, VIEW __VA_OPT__(,) __VA_ARGS__) \
  MUON_EACH_NODE_STEM(emit __VA_OPT__(,) __VA_ARGS__)

// Expands to extern VALUE cNode, etc.
#define EMIT(Title, lower, UPPER) extern VALUE c##Title;
EACH_ABSTRACT_NODE_STEM(EMIT)
#undef EMIT

// Expands to extern const rb_data_type_t node_type, etc.
#define EMIT(Title, lower, UPPER) extern const rb_data_type_t lower##_type;
EACH_ABSTRACT_NODE_STEM(EMIT)
#undef EMIT

// Expands to MuonNode *as_muon_node(VALUE rb_node) { ... }, etc.
#define EMIT(Title, lower, U) [[gnu::returns_nonnull]] \
  static inline Muon##Title *as_muon_##lower(VALUE rb_##lower) { \
    Muon##Title *lower; \
    TypedData_Get_Struct(rb_##lower, Muon##Title, &lower##_type, lower); \
    return lower; \
  }
EACH_ABSTRACT_NODE_STEM(EMIT)
#undef EMIT

// Concrete as_* functions: as_access_expr, as_define_stmt, etc.
#define EMIT(Title, lower, U) [[gnu::nonnull]] \
  static inline VALUE as_##lower(Muon##Title *lower) { \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wcast-qual\"") \
    auto data = (typeof_unqual(Muon##Title) *) lower; \
    _Pragma("GCC diagnostic pop") \
    return TypedData_Wrap_Struct(c##Title, &lower##_type, data); \
  }
MUON_EACH_NODE_STEM(EMIT)
#undef EMIT

[[gnu::nonnull]] static inline VALUE as_expr(MuonExpr *expr) {
  switch (expr->kind) {
#define EMIT(Title, lower, UPPER) case MUON_##UPPER##_EXPR: \
      return as_##lower##_expr((Muon##Title##Expr *) expr);
    MUON_EACH_EXPR_STEM(EMIT)
#undef EMIT
  }
}

[[gnu::nonnull]] static inline VALUE as_sign(MuonSign *sign) {
  switch (sign->kind) {
#define EMIT(Title, lower, UPPER) case MUON_##UPPER##_SIGN: \
      return as_##lower##_sign((Muon##Title##Sign *) sign);
    MUON_EACH_SIGN_STEM(EMIT)
#undef EMIT
  }
}

[[gnu::nonnull]] static inline VALUE as_stmt(MuonStmt *stmt) {
  switch (stmt->kind) {
#define EMIT(Title, lower, UPPER) case MUON_##UPPER##_STMT: \
      return as_##lower##_stmt((Muon##Title##Stmt *) stmt);
    MUON_EACH_STMT_STEM(EMIT)
#undef EMIT
  }
}

[[gnu::nonnull]] static inline VALUE as_view(MuonView *view) {
  switch (view->kind) {
#define EMIT(Title, lower, UPPER) case MUON_##UPPER##_VIEW: \
      return as_##lower##_view((Muon##Title##View *) view);
    MUON_EACH_VIEW_STEM(EMIT)
#undef EMIT
  }
}

[[gnu::nonnull]] static inline VALUE as_node(MuonNode *node) {
  switch (node->kind) {
#define EMIT(Title, lower, UPPER) \
      case MUON_##UPPER: return as_##lower((Muon##Title *) node);
    MUON_EACH_NODE_STEM(EMIT)
#undef EMIT
  }
}

void Init_muon_node(VALUE mMuon);

#endif /* MUON_RUBY_NODE_H */
