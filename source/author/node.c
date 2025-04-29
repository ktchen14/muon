#include "common.h"

#include "../stator.h"
#include "../inductor.h"

#include <llvm-c/Types.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static LLVMValueRef SKIP = (void *) &(int) {1};

static char name[256];

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

  snprintf(name, sizeof(name), "access.%zu", expr->as_node.id);
  LLVMValueRef lambda = LLVMAddFunction(author->module, name, lambda_ty);
  LLVMBasicBlockRef main = LLVMAppendBasicBlock(lambda, "main.0");
  LLVMBuilderRef tail = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(tail, main);

  LLVMValueRef argument = LLVMGetParam(lambda, 0);
  LLVMValueRef result = LLVMBuildExtractValue(tail, argument, 0, "");
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
  const mu_node_t *operator = &expr->operator->as_node;

  LLVMValueRef operator_val = evince_result(author, operator);

  const mu_type_t *operator_type;
  if ((operator_type = evince_type(author->inductor, operator)) == NULL)
    return NULL;

  LLVMTypeRef operator_ty;
  if ((operator_ty = get_type(author, operator_type)) == NULL)
    return NULL;

  const mu_node_t *argument = &expr->argument->as_node;

  LLVMValueRef argument_val = evince_result(author, argument);

  const mu_type_t *argument_type;
  if ((argument_type = evince_type(author->inductor, argument)) == NULL)
    return NULL;

  LLVMTypeRef argument_ty;
  if ((argument_ty = get_type(author, argument_type)) == NULL)
    return NULL;

  unsigned int argc = LLVMCountParamTypes(operator_ty);
  assert(argc == 1);
  LLVMValueRef argv[argc];
  argv[0] = argument_val;

  snprintf(name, sizeof(name), "invoke.%zu", expr->as_node.id);
  return LLVMBuildCall2(
      author->tail, operator_ty, operator_val, argv, argc, name);
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
  return evince_result(author, target);
}

__attribute__((nonnull))
static LLVMValueRef native_expr_emit(author_t *author, const mu_native_expr_t *expr) {
  return SKIP;
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
  LLVMTypeRef allocation_type;
  if ((allocation_type = LLVMArrayType2(matter_type, expr->argc)) == NULL)
    return NULL;

  // Calculate the size of allocation_type
  size_t allocation_size = LLVMABISizeOfType(author->layout, allocation_type);

  // %size = size_t <allocation_size>
  LLVMValueRef size;
  if ((size = LLVMConstInt(author->size_type, allocation_size, 0)) == NULL)
    return NULL;

  // %allocation = call ptr @malloc(size_t %size)
  LLVMValueRef malloc_argv[] = { size };
  LLVMValueRef allocation = LLVMBuildCall2(
      author->tail, author->malloc_type, author->malloc, malloc_argv, 1, "");
  if (allocation == NULL)
    return NULL;

  // %length = size_t <expr->argc>
  LLVMValueRef length;
  if ((length = LLVMConstInt(author->size_type, expr->argc, 0)) == NULL)
    return NULL;

  // %none = ptr poison
  LLVMValueRef none;
  if ((none = LLVMGetPoison(author->star_type)) == NULL)
    return NULL;

  // %result = { size_t, ptr } { size_t %length, ptr %none }
  LLVMValueRef struct_argv[] = { length, none };
  LLVMValueRef result;
  if ((result = LLVMConstStruct(struct_argv, 2, 0)) == NULL)
    return NULL;

  // %5 = insertvalue { size_t, ptr } %result, ptr %allocation, 1
  if ((result = LLVMBuildInsertValue(author->tail, result, allocation, 1, "")) == NULL)
    return NULL;

  for (size_t i = 0; i < expr->argc; i++) {
    LLVMValueRef argument = evince_result(author, &expr->argv[i]->as_node);

    // %index = size_t <i>
    LLVMValueRef index;
    if ((index = LLVMConstInt(author->size_type, i, 0)) == NULL)
      return NULL;

    // %target = getelementptr inbounds nuw %allocation_type, ptr %allocation,
    //           size_t %index
    LLVMValueRef target = LLVMBuildGEPWithNoWrapFlags(
        author->tail,
        allocation_type,
        allocation,
        (LLVMValueRef[]) { index },
        1,
        "",
        LLVMGEPFlagInBounds | LLVMGEPFlagNUW);
    if (target == NULL)
      return NULL;

    // store i64 2, ptr %7, align 4
    if (LLVMBuildStore(author->tail, argument, target) == NULL)
      return NULL;
  }

  return result;
}

LLVMValueRef node_emit(author_t *author, const mu_node_t *node) {
  switch (node->kind) {
#define MU_EMIT(lower, upper, t) case MU_##upper##_EXPR: \
      return lower##_expr_emit(author, (const mu_##lower##_expr_t *) node);
    MU_EACH_EXPR_KIND(MU_EMIT);
#undef MU_EMIT

    default: return SKIP;
  }
}

LLVMModuleRef script_emit(induce_t *induce, const mu_node_t *root) {
  author_t author;
  if (author_initialize(&author, induce->detect, induce) == NULL)
    return NULL;

  LLVMTypeRef initialize_type = LLVMFunctionType(LLVMVoidType(), NULL, 0, 0);
  LLVMValueRef lambda = LLVMAddFunction(author.module, "initialize", initialize_type);
  LLVMBasicBlockRef b = LLVMAppendBasicBlock(lambda, "");
  LLVMBuilderRef tail = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(tail, b);

  if (author_continue(&author, lambda, tail) == NULL)
    return NULL;

  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL) {
      const mu_lambda_expr_t *lambda_expr;

      if ((lambda_expr = mu_node_cast(next, lambda_expr)) != NULL) {
        const mu_type_t *lambda_type = evince_type(author.inductor, &lambda_expr->as_node);
        LLVMTypeRef lambda_ty;
        if ((lambda_ty = get_type(&author, lambda_type)) == NULL)
          return NULL;

        snprintf(name, sizeof(name), "lambda.%zu", lambda_expr->as_node.id);
        LLVMValueRef lambda = LLVMAddFunction(author.module, name, lambda_ty);
        LLVMBasicBlockRef entry = LLVMAppendBasicBlock(lambda, "");
        LLVMBuilderRef tail = LLVMCreateBuilder();
        LLVMPositionBuilderAtEnd(tail, entry);

        if (author_continue(&author, lambda, tail) == NULL)
          return NULL;
      }

      node = node_continue(node, next);
    }

    LLVMValueRef result;

    if ((result = node_emit(&author, node)) == NULL)
      return NULL;
    if (result == SKIP)
      continue;
    author.node_to_value[node->id] = result;

    const mu_coercion_t *coercion;
    const mu_type_t *target_type;
    if ((coercion = evince_coercion(induce, node, &target_type)) != NULL) {
      const mu_type_t *source_muon_type = evince_type(induce, node);
      LLVMTypeRef source_type = get_type(&author, source_muon_type);
      assert(source_type != NULL);

      const mu_type_t *target_muon_type = target_type;
      LLVMTypeRef target_type = get_type(&author, target_muon_type);
      assert(target_type != NULL);

      LLVMValueRef source = result;

      info_t info = { .source_type = source_muon_type, .source = source };
      if ((result = coercion_emit(&author, coercion, info)) == NULL)
        return NULL;
      author.node_to_value[node->id] = result;
    }
  } while ((node = node_return(node)) != NULL);

  assert(author.stream_length == 1);
  LLVMBuildRetVoid(author.tail);
  LLVMDisposeBuilder(author.tail);

  return author.module;
}

/* int main(int argc, char const *argv[]) { */
/*   LLVMModuleRef module = LLVMModuleCreateWithName("my_module"); */

/*   LLVMTypeRef param_types[] = { LLVMInt32Type(), LLVMInt32Type() }; */
/*   LLVMTypeRef return_type = LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0); */
/*   LLVMValueRef sum = LLVMAddFunction(module, "sum", return_type); */

/*   LLVMBasicBlockRef entry = LLVMAppendBasicBlock(sum, "entry"); */

/*   LLVMBuilderRef builder = LLVMCreateBuilder(); */
/*   LLVMPositionBuilderAtEnd(builder, entry); */
/*   LLVMValueRef tmp = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "tmp"); */
/*   LLVMBuildRet(builder, tmp); */

/*   char *error = NULL; */
/*   LLVMVerifyModule(module, LLVMAbortProcessAction, &error); */
/*   LLVMDisposeMessage(error); */

/*   LLVMExecutionEngineRef engine; */
/*   error = NULL; */
/*   LLVMLinkInMCJIT(); */
/*   LLVMInitializeNativeTarget(); */
/*   LLVMInitializeNativeAsmPrinter(); */
/*   if (LLVMCreateExecutionEngineForModule(&engine, module, &error) != 0) { */
/*     fprintf(stderr, "failed to create execution engine\n"); */
/*     abort(); */
/*   } */
/*   if (error) { */
/*     fprintf(stderr, "error: %s\n", error); */
/*     LLVMDisposeMessage(error); */
/*     exit(EXIT_FAILURE); */
/*   } */

/*   if (argc < 3) { */
/*     fprintf(stderr, "usage: %s x y\n", argv[0]); */
/*     exit(EXIT_FAILURE); */
/*   } */
/*   long long x = strtoll(argv[1], NULL, 10); */
/*   long long y = strtoll(argv[2], NULL, 10); */

/*   int (*sum_func)(int, int) = (int (*)(int, int)) LLVMGetFunctionAddress(engine, "sum"); */
/*   printf("%d\n", sum_func(x, y)); */

/*   // Write out bitcode to file */
/*   if (LLVMWriteBitcodeToFile(module, "sum.bc") != 0) { */
/*     fprintf(stderr, "error writing bitcode to file, skipping\n"); */
/*   } */

/*   LLVMDisposeBuilder(builder); */
/*   LLVMDisposeExecutionEngine(engine); */
/* } */
