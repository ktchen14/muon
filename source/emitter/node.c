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

static LLVMValueRef evince_result(const frame_t *frame, const mu_node_t *node)
  __attribute__((nonnull, returns_nonnull));

void write_script(
    mu_engine_t *engine,
    induce_t *induce,
    const mu_node_t *root) {
  const mu_node_t *node = root, *next;
  do {
    while ((next = node_at(node, node_cursor(node)->i++)) != NULL)
      node = node_continue(node, next);
  } while ((node = node_return(node)) != NULL);
}

__attribute__((nonnull)) static LLVMValueRef boolean_expr_frame(
    frame_t *frame, const mu_boolean_expr_t *expr) {
  return LLVMConstInt(LLVMInt1Type(), expr->data, 0);
}

__attribute__((nonnull)) static LLVMValueRef cast_expr_induce(
    frame_t *frame, const mu_cast_expr_t *expr) {
  return evince_result(frame, &expr->matter->as_node);
}

__attribute__((nonnull)) static LLVMValueRef integer_expr_frame(
    frame_t *frame, const mu_integer_expr_t *expr) {
  return LLVMConstInt(LLVMInt64Type(), expr->data, 0);
}

__attribute__((nonnull)) static LLVMValueRef invoke_expr_frame(
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
static frame_t *lambda_expr_frame(frame_t *frame, const mu_lambda_expr_t *expr) {
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
static LLVMValueRef name_expr_frame(frame_t *frame, const mu_name_expr_t *expr) {
  emitter_t *emitter = frame->emitter;

  const mu_node_t *target = detect_evince(emitter->detect, &expr->as_node);
  assert(target != NULL);
  return evince_result(frame, target);
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
