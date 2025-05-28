#include "common.h"

#include "../common.h"
#include "../engine.h"
#include "../inductor.h"

#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/Error.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/PassBuilder.h>
#include <llvm-c/Types.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INTERNAL_STRING(string) #string
#define INDIRECT_STRING(string) INTERNAL_STRING(string)
#define ID INDIRECT_STRING(SIZE_MAX)

LLVMValueRef SKIP = (void *) &(int) {1};

static LLVMValueRef evince_result(const author_t *author, const mu_node_t *node) {
  assert(node->id < author->node_length);
  LLVMValueRef result = author->node_to_value[node->id];
  assert(result != NULL);
  return result;
}

__attribute__((nonnull)) static LLVMValueRef access_expr_emit(
    author_t *author, const mu_access_expr_t *expr) {
  const mu_type_t *lambda_type = evince_type(author->inductor, &expr->as_node);
  LLVMTypeRef lambda_ty;
  if ((lambda_ty = get_type(author, lambda_type)) == NULL)
    return NULL;

  char name[sizeof("access." ID)];
  int e = snprintf(name, sizeof(name), "access.%zu", expr->as_node.id);
  assert((size_t) e < sizeof(name));

  LLVMValueRef lambda = LLVMAddFunction(author->module, name, lambda_ty);
  LLVMSetLinkage(lambda, LLVMPrivateLinkage);
  LLVMSetUnnamedAddress(lambda, LLVMLocalUnnamedAddr);
  LLVMBasicBlockRef main = LLVMAppendBasicBlock(lambda, "");

  LLVMBuilderRef tail = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(tail, main);

  LLVMValueRef argument = LLVMGetParam(lambda, 0);
  LLVMValueRef result = LLVMBuildExtractValue(tail, argument, 0, "result");
  LLVMBuildRet(tail, result);
  LLVMDisposeBuilder(tail);

  return lambda;
}

__attribute__((nonnull)) static LLVMValueRef boolean_expr_emit(
    author_t *author, const mu_boolean_expr_t *expr) {
  return LLVMConstInt(author->bool_type, expr->data, 0);
}

__attribute__((nonnull)) static LLVMValueRef cast_expr_emit(
    author_t *author, const mu_cast_expr_t *expr) {
  return evince_result(author, &expr->matter->as_node);
}

__attribute__((nonnull)) static LLVMValueRef integer_expr_emit(
    author_t *author, const mu_integer_expr_t *expr) {
  return LLVMConstInt(LLVMInt64Type(), expr->data, 0);
}

__attribute__((nonnull)) static LLVMValueRef invoke_expr_emit(
    author_t *author, const mu_invoke_expr_t *expr) {
  const mu_node_t *operator_node = &expr->operator->as_node;

  LLVMValueRef operator = evince_result(author, operator_node);

  const mu_type_t *operator_muon_type;
  if ((operator_muon_type = evince_type(author->inductor, operator_node)) == NULL)
    return NULL;

  LLVMTypeRef operator_type;
  if ((operator_type = get_type(author, operator_muon_type)) == NULL)
    return NULL;

  const mu_node_t *argument = &expr->argument->as_node;

  LLVMValueRef argument_val = evince_result(author, argument);

  const mu_type_t *argument_type;
  if ((argument_type = evince_type(author->inductor, argument)) == NULL)
    return NULL;

  LLVMTypeRef argument_ty;
  if ((argument_ty = get_type(author, argument_type)) == NULL)
    return NULL;

  unsigned int argc = LLVMCountParamTypes(operator_type);
  assert(argc == 1);
  LLVMValueRef argv[argc];
  argv[0] = argument_val;

  char name[sizeof("invoke." ID)];
  int e = snprintf(name, sizeof(name), "invoke.%zu", expr->as_node.id);
  assert((size_t) e < sizeof(name));
  return LLVMBuildCall2(
      author->tail, operator_type, operator, argv, argc, name);
}

__attribute__((nonnull))
static LLVMValueRef lambda_expr_emit(author_t *author, const mu_lambda_expr_t *expr) {
  LLVMValueRef matter = evince_result(author, &expr->matter->as_node);
  LLVMBuildRet(author->tail, matter);
  LLVMDisposeBuilder(author->tail);
  return author_return(author);
}

__attribute__((nonnull))
static LLVMValueRef name_expr_emit(author_t *author, const mu_name_expr_t *expr) {
  const mu_node_t *target = detect_evince(author->detect, &expr->as_node);
  assert(target != NULL);

  LLVMValueRef variable = evince_result(author, target);
  if (!LLVMIsAGlobalVariable(variable))
    return variable;

  LLVMTypeRef data_type = LLVMGlobalGetValueType(variable);

  // %name.[id] = load <data_type>, %variable
  char name[sizeof("name." ID)];
  int e = snprintf(name, sizeof(name), "name.%zu", expr->as_node.id);
  assert((size_t) e < sizeof(name));
  return LLVMBuildLoad2(author->tail, data_type, variable, name);
}

__attribute__((nonnull))
static LLVMValueRef native_expr_emit(author_t *author, const mu_native_expr_t *expr) {
  assert(author->native_expr_emit != NULL);
  return author->native_expr_emit(author, expr);
}

__attribute__((nonnull))
static LLVMValueRef record_expr_emit(author_t *author, const mu_record_expr_t *expr) {
  const mu_type_t *type = evince_type(author->inductor, &expr->as_node);
  LLVMTypeRef ty;
  if ((ty = get_type(author, type)) == NULL)
    return NULL;

  LLVMValueRef result;
  if ((result = LLVMGetPoison(ty)) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    const mu_expr_member_t *member = expr->argv[i];
    const mu_expr_t *matter = member->expr;

    LLVMValueRef argument = evince_result(author, &matter->as_node);
    result = LLVMBuildInsertValue(author->tail, result, argument, i, "");
  }

  return result;
}

__attribute__((nonnull))
static LLVMValueRef sequence_expr_emit(author_t *author, const mu_sequence_expr_t *expr) {
  return SKIP;
}

__attribute__((nonnull))
static LLVMValueRef switch_expr_emit(author_t *author, const mu_switch_expr_t *expr) {
  return SKIP;
}

__attribute__((nonnull))
static LLVMValueRef vector_expr_emit(author_t *author, const mu_vector_expr_t *expr) {
  const mu_type_t *type = evince_type(author->inductor, &expr->as_node);
  const mu_core_type_t *vector_type = mu_type_cast(type, vector_type);
  assert(vector_type != NULL);
  assert(vector_type->core == vector_type->as_type.induce->vector_core);
  const mu_type_t *matter_muon_type = vector_type->argv[0];

  // Type of the vector expr itself. Should be { i64, ptr }.
  LLVMTypeRef ty;
  if ((ty = get_type(author, type)) == NULL)
    return NULL;

  // %matter_type = type <matter_muon_type>
  LLVMTypeRef matter_type = get_type(author, matter_muon_type);

  // %allocation_type = type [<expr->argc> x %matter_type]
  LLVMTypeRef allocation_type = LLVMArrayType2(matter_type, expr->argc);

  // Calculate the size of allocation_type
  size_t allocation_size = LLVMABISizeOfType(author->layout, allocation_type);

  // %size = size_t <allocation_size>
  LLVMValueRef size = LLVMConstInt(author->size_type, allocation_size, 0);

  // <name> = "vector.[id].allocation"
  char allocation_name[sizeof("vector." ID ".allocation")];
  int e = snprintf(allocation_name, sizeof(allocation_name), "vector.%zu.allocation", expr->as_node.id);
  assert((size_t) e < sizeof(allocation_name));

  // %allocation = call ptr @malloc(size_t %size)
  LLVMValueRef malloc_argv[] = { size };
  LLVMValueRef allocation = LLVMBuildCall2(
      author->tail, author->malloc_type, author->malloc, malloc_argv, 1, allocation_name);

  // %length = size_t <expr->argc>
  LLVMValueRef length = LLVMConstInt(author->size_type, expr->argc, 0);

  // %none = ptr poison
  LLVMValueRef none;
  if ((none = LLVMGetPoison(author->star_type)) == NULL)
    return NULL;

  // %result = { size_t, ptr } { size_t %length, ptr %none }
  LLVMValueRef struct_argv[] = { length, none };
  LLVMValueRef result = LLVMConstStruct(struct_argv, 2, 0);

  // <name> = "vector.[id]"
  char name[sizeof("vector." ID)];
  e = snprintf(name, sizeof(name), "vector.%zu", expr->as_node.id);
  assert((size_t) e < sizeof(name));

  // %5 = insertvalue { size_t, ptr } %result, ptr %allocation, 1
  result = LLVMBuildInsertValue(author->tail, result, allocation, 1, name);

  for (size_t i = 0; i < expr->argc; i++) {
    LLVMValueRef argument = evince_result(author, &expr->argv[i]->as_node);

    // %index = size_t <i>
    LLVMValueRef index = LLVMConstInt(author->size_type, i, 0);

    char name[sizeof("vector." ID ".allocation." ID)];
    int e = snprintf(
        name, sizeof(name), "vector.%zu.allocation.%zu", expr->as_node.id, i);
    assert((size_t) e < sizeof(name));

    // %target = getelementptr inbounds nuw %allocation_type, ptr %allocation,
    //           size_t %index
    LLVMValueRef target = LLVMBuildGEPWithNoWrapFlags(
        author->tail,
        allocation_type,
        allocation,
        (LLVMValueRef[]) { author->zero_size, index },
        2,
        name,
        LLVMGEPFlagInBounds | LLVMGEPFlagNUW);

    // store i64 2, ptr %7, align 4
    LLVMBuildStore(author->tail, argument, target);
  }

  return result;
}

__attribute__((nonnull))
static LLVMValueRef define_stmt_emit(author_t *author, const mu_define_stmt_t *stmt) {
  LLVMValueRef expr_result = evince_result(author, &stmt->expr->as_node);
  LLVMTypeRef expr_type = LLVMTypeOf(expr_result);

  char name[256];
  snprintf(name, sizeof(name), "muon.%s", stmt->name->text);

  // @result = global <expr_type> poison
  LLVMValueRef result = LLVMAddGlobal(author->module, expr_type, name);
  LLVMSetInitializer(result, LLVMGetPoison(expr_type));

  // store <expr_type> <expr_result>, @result
  LLVMBuildStore(author->tail, expr_result, result);

  return result;
}

LLVMValueRef node_emit(author_t *author, const mu_node_t *node) {
  switch ON_ABSTRACT_OBJECT(node) {
#define MU_EMIT(lower, upper, t) case MU_##upper##_EXPR: \
      return lower##_expr_emit(author, (const mu_##lower##_expr_t *) node);
    MU_EACH_EXPR_KIND(MU_EMIT);
#undef MU_EMIT

    case IS_KIND_OF(define_stmt):
      return define_stmt_emit(author, define_stmt);

    default: return SKIP;
  }
}

LLVMModuleRef script_emit(author_t *author, const mu_node_t *root, const char *source_name) {
  LLVMTypeRef initialize_type = LLVMFunctionType(LLVMVoidType(), NULL, 0, 0);
  LLVMValueRef lambda = LLVMAddFunction(author->module, "initialize", initialize_type);
  LLVMBasicBlockRef b = LLVMAppendBasicBlock(lambda, "");
  LLVMBuilderRef tail = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(tail, b);

  if (author_continue(author, lambda, tail) == NULL)
    return NULL;

  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL) {
      const mu_lambda_expr_t *lambda_expr;

      if ((lambda_expr = mu_node_cast(next, lambda_expr)) != NULL) {
        const mu_type_t *lambda_type = evince_type(author->inductor, &lambda_expr->as_node);
        LLVMTypeRef lambda_ty;
        if ((lambda_ty = get_type(author, lambda_type)) == NULL)
          return NULL;

        char name[sizeof("lambda." ID)];
        int e = snprintf(name, sizeof(name), "lambda.%zu", lambda_expr->as_node.id);
        assert((size_t) e < sizeof(name));
        LLVMValueRef lambda = LLVMAddFunction(author->module, name, lambda_ty);

        LLVMBasicBlockRef main = LLVMAppendBasicBlock(lambda, "");
        LLVMBuilderRef tail = LLVMCreateBuilder();
        LLVMPositionBuilderAtEnd(tail, main);

        if (author_continue(author, lambda, tail) == NULL)
          return NULL;
      }

      node = node_continue(node, next);
    }

    LLVMValueRef result;

    if ((result = node_emit(author, node)) == NULL)
      return NULL;
    if (result == SKIP)
      continue;
    author->node_to_value[node->id] = result;

    const mu_coercion_t *coercion;
    const mu_type_t *target_type;
    if ((coercion = evince_coercion(author->inductor, node, &target_type)) != NULL) {
      const mu_type_t *source_muon_type = evince_type(author->inductor, node);
      LLVMTypeRef source_type = get_type(author, source_muon_type);
      assert(source_type != NULL);

      const mu_type_t *target_muon_type = target_type;
      LLVMTypeRef target_type = get_type(author, target_muon_type);
      assert(target_type != NULL);

      LLVMValueRef source = result;

      info_t info = { .source_type = source_muon_type, .source = source };
      if ((result = coercion_emit(author, coercion, info)) == NULL)
        return NULL;
      author->node_to_value[node->id] = result;
    }
  } while ((node = node_return(node)) != NULL);

  assert(author->stream_length == 1);
  LLVMBuildRetVoid(author->tail);
  LLVMDisposeBuilder(author->tail);

  // This can't fail absent a bug in Muon so just abort() on failure
  LLVMVerifyModule(author->module, LLVMAbortProcessAction, NULL);
  LLVMSetSourceFileName(author->module, source_name, strlen(source_name));

  LLVMPassBuilderOptionsRef option = LLVMCreatePassBuilderOptions();
  LLVMErrorRef e;
  e = LLVMRunPasses(author->module, "default<O2>", NULL, option);
  if (e != NULL)
    abort();
  LLVMDisposePassBuilderOptions(option);

  LLVMWriteBitcodeToFile(author->module, "module.bc");
  LLVMPrintModuleToFile(author->module, "module.ll", NULL);

  return author->module;
}
