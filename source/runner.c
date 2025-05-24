#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/Error.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/LLJIT.h>
#include <llvm-c/Orc.h>
#include <llvm-c/Support.h>
#include <llvm-c/Target.h>
#include <llvm-c/Types.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void mu_run(LLVMModuleRef module) {
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();

  // Create a JIT builder
  LLVMOrcLLJITBuilderRef builder = NULL;
  builder = LLVMOrcCreateLLJITBuilder();

  // Create the JIT
  LLVMOrcLLJITRef jit;
  LLVMErrorRef e;
  e = LLVMOrcCreateLLJIT(&jit, builder);
  LLVMCantFail(e);

  // Get the main dylib
  LLVMOrcJITDylibRef dylib = LLVMOrcLLJITGetMainJITDylib(jit);

  // Create the generator to find symbols in the current process
  LLVMOrcDefinitionGeneratorRef generator;
  e = LLVMOrcCreateDynamicLibrarySearchGeneratorForProcess(&generator,
      LLVMOrcLLJITGetGlobalPrefix(jit), 0, NULL);
  LLVMCantFail(e);
  LLVMOrcJITDylibAddGenerator(dylib, generator);

  // Create a thread safe context
  LLVMOrcThreadSafeContextRef ts_context = LLVMOrcCreateNewThreadSafeContext();

  // Create a thread safe module
  LLVMOrcThreadSafeModuleRef ts_module = LLVMOrcCreateNewThreadSafeModule(
      module, ts_context);

  // Add the IR module
  e = LLVMOrcLLJITAddLLVMIRModule(jit, dylib, ts_module);
  LLVMCantFail(e);

  LLVMOrcExecutorAddress addr;
  e = LLVMOrcLLJITLookup(jit, &addr, "initialize");
  LLVMCantFail(e);

  void (*invoke)(void) = (void (*)(void)) addr;
  invoke();

  e = LLVMOrcLLJITLookup(jit, &addr, "muon.result");
  LLVMCantFail(e);
  uint64_t result;
  memcpy(&result, (void *) addr, sizeof(uint64_t));
  fprintf(stderr, "result = %" PRIu64 "\n", result);

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
