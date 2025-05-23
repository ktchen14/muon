#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/Error.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/LLJIT.h>
#include <llvm-c/Orc.h>
#include <llvm-c/Support.h>
#include <llvm-c/Target.h>
#include <llvm-c/Types.h>

void mu_run(LLVMModuleRef module) {
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();

  LLVMOrcThreadSafeContextRef tsc = LLVMOrcCreateNewThreadSafeContext();
  LLVMOrcThreadSafeModuleRef tsm = LLVMOrcCreateNewThreadSafeModule(module, tsc);

  LLVMOrcLLJITBuilderRef jit_builder = LLVMOrcCreateLLJITBuilder();
  LLVMOrcLLJITRef jit;
  LLVMErrorRef e = LLVMOrcCreateLLJIT(&jit, jit_builder);
  LLVMCantFail(e);

  LLVMOrcJITDylibRef dylib = LLVMOrcLLJITGetMainJITDylib(jit);

  e = LLVMOrcLLJITAddLLVMIRModule(jit, dylib, tsm);
  LLVMCantFail(e);

  LLVMOrcExecutorAddress call;
  e = LLVMOrcLLJITLookup(jit, &call, "initialize");
  LLVMCantFail(e);

  // LLVMLinkInMCJIT();
  // LLVMExecutionEngineRef exec_engine;
  // error = NULL;
  // if (LLVMCreateExecutionEngineForModule(&exec_engine, module, &error) != 0) {
  //   fprintf(stderr, "failed to create execution engine\n");
  //   abort();
  // }
  // if (error) {
  //   fprintf(stderr, "error: %s\n", error);
  //   LLVMDisposeMessage(error);
  //   exit(EXIT_FAILURE);
  // }
  // LLVMAddGlobalMapping(exec_engine, author->malloc, (void *) test);

  // LLVMValueRef module_initialize;
  // LLVMFindFunction(exec_engine, "initialize", &module_initialize);
  // LLVMRunFunction(exec_engine, module_initialize, 0, NULL);

}
