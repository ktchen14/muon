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

static LLVMValueRef evince_result(const frame_t *frame, const mu_node_t *node) {
  emitter_t *emitter = frame->emitter;
  assert(node->id < emitter->node_length);
  LLVMValueRef result = emitter->node_to_value[node->id];
  assert(result != NULL);
  return result;
}

__attribute__((nonnull)) static LLVMValueRef boolean_expr_emit(
    frame_t *frame, const mu_boolean_expr_t *expr) {
  return LLVMConstInt(LLVMInt1Type(), expr->data, 0);
}

__attribute__((nonnull)) static LLVMValueRef cast_expr_emit(
    frame_t *frame, const mu_cast_expr_t *expr) {
  return evince_result(frame, &expr->matter->as_node);
}

__attribute__((nonnull)) static LLVMValueRef integer_expr_emit(
    frame_t *frame, const mu_integer_expr_t *expr) {
  return LLVMConstInt(LLVMInt64Type(), expr->data, 0);
}

__attribute__((nonnull)) static LLVMValueRef invoke_expr_emit(
    frame_t *frame, const mu_invoke_expr_t *expr) {
  emitter_t *emitter = frame->emitter;

  const mu_node_t *operator = &expr->operator->as_node;

  LLVMValueRef operator_val = evince_result(frame, operator);

  const mu_type_t *operator_type;
  if ((operator_type = evince_type(emitter->inductor, operator)) == NULL)
    return NULL;

  LLVMTypeRef operator_ty;
  if ((operator_ty = get_type(emitter, operator_type)) == NULL)
    return NULL;

  const mu_node_t *argument = &expr->argument->as_node;

  LLVMValueRef argument_val = evince_result(frame, argument);

  const mu_type_t *argument_type;
  if ((argument_type = evince_type(emitter->inductor, argument)) == NULL)
    return NULL;

  LLVMTypeRef argument_ty;
  if ((argument_ty = get_type(emitter, argument_type)) == NULL)
    return NULL;

  LLVMTypeRef return_type = LLVMGetReturnType(operator_ty);
  size_t argc = LLVMCountParamTypes(operator_ty);
  assert(argc == 0);
  LLVMValueRef argv[argc];
  argv[0] = argument_val;

  return LLVMBuildCall2(
      frame->builder, return_type, operator_val, argv, argc, "invoke");
}

__attribute__((nonnull))
static frame_t *lambda_expr_emit(frame_t *frame, const mu_lambda_expr_t *expr) {
  emitter_t *emitter = frame->emitter;

  const mu_type_t *lambda_type = evince_type(emitter->inductor, &expr->as_node);
  LLVMTypeRef lambda_ty;
  if ((lambda_ty = get_type(emitter, lambda_type)) == NULL)
    return NULL;

  LLVMValueRef lambda = LLVMAddFunction(emitter->module, "lambda", lambda_ty);
  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(lambda, "");
  LLVMBuilderRef builder = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(builder, entry);

  frame_t newframe = { .emitter = emitter, .lambda = lambda, .builder = builder };
  emitter->frame[emitter->frame_length++] = newframe;
  return &emitter->frame[emitter->frame_length - 1];
}

__attribute__((nonnull))
static LLVMValueRef name_expr_emit(frame_t *frame, const mu_name_expr_t *expr) {
  emitter_t *emitter = frame->emitter;

  const mu_node_t *target = detect_evince(emitter->detect, &expr->as_node);
  assert(target != NULL);
  return evince_result(frame, target);
}

__attribute__((nonnull))
static LLVMValueRef vector_expr_emit(frame_t *frame, const mu_vector_expr_t *expr) {
}

const LLVMValueRef *node_emit(frame_t *frame, const mu_expr_t *expr) {
  switch (expr->kind) {
#define MU_EMIT(lower, upper, t) case MU_##upper##_EXPR: \
      return lower##_expr_emit(frame, (const mu_##lower##_expr_t *) expr);
    MU_EACH_EXPR_KIND(MU_EMIT);
#undef MU_EMIT
  }
}

void script_emit(
    mu_engine_t *engine, induce_t *induce, const mu_node_t *root) {
  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);
  } while ((node = node_return(node)) != NULL);
}

/* const LLVMValueRef *induce_node(induce_t *induce, const mu_node_t *root) { */
/*   assert(induce->scheme == NULL); */
/*   induce->scheme = &(mu_scheme_t) { .induce = induce, .id = induce->type_number }; */

/*   assert(root->id < induce->node_length); */

/*   const mu_node_t *node = root, *next; */
/*   do { */
/*     while ((next = node_at(node, node_cursor(node)->i++)) != NULL) { */
/*       node = node_continue(node, next); */

/*       const mu_datatype_stmt_t *datatype_stmt; */
/*       if ((datatype_stmt = mu_node_cast(node, datatype_stmt)) != NULL) { */
/*         const mu_core_t *core; */
/*         if ((core = mu_simple_core(induce, datatype_stmt->name)) == NULL) */
/*           return NULL; */
/*         induce->core[induce->core_length++] = core; */
/*         induce->datatype_core = core; */
/*       } */

/*       if (node->kind != MU_DEFINE_STMT_NODE) */
/*         continue; */

/*       mu_scheme_t *scheme; */
/*       if ((scheme = mu_scheme(induce->scheme)) == NULL) */
/*         return NULL; */
/*       induce->scheme = scheme; */
/*     } */

/*     // Induce the type of the node */
/*     const mu_type_t *type; */
/*     if ((type = node_induce(induce, node)) == NULL) */
/*       return NULL; */

/*     if (node->kind == MU_DEFINE_STMT_NODE) { */
/*       mu_scheme_t *parent = induce->scheme->parent; */
/*       free(induce->scheme); */
/*       induce->scheme = parent; */
/*     } */

/*     induce->node_to_type[node->id] = type; */
/*   } while ((node = node_return(node)) != NULL); */

/*   return evince_type(induce, root); */
/* } */

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
