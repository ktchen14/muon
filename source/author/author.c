#include "common.h"

#include "../inductor.h"

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>

#include <stddef.h>
#include <stdlib.h>

author_t *author_initialize(
    author_t *author, const detect_result_t *detect, mu_inductor_t *inductor) {
  LLVMInitializeNativeTarget(); 

  char *triple;
  if ((triple = LLVMGetDefaultTargetTriple()) == NULL)
    goto except_triple;

  LLVMTargetRef target;
  if (LLVMGetTargetFromTriple(triple, &target, NULL) != 0)
    goto except_target;

  LLVMTargetMachineOptionsRef option;
  if ((option = LLVMCreateTargetMachineOptions()) == NULL)
    goto except_option;
  LLVMTargetMachineOptionsSetCodeGenOptLevel(option, LLVMCodeGenLevelDefault);
  LLVMTargetMachineOptionsSetRelocMode(option, LLVMRelocDefault);
  LLVMTargetMachineOptionsSetCodeModel(option, LLVMCodeModelDefault);

  char *cpu;
  if ((cpu = LLVMGetHostCPUName()) == NULL)
    goto except_cpu;
  LLVMTargetMachineOptionsSetCPU(option, cpu);

  char *features;
  if ((features = LLVMGetHostCPUFeatures()) == NULL)
    goto except_features;
  LLVMTargetMachineOptionsSetFeatures(option, features);

  LLVMTargetMachineRef machine = LLVMCreateTargetMachineWithOptions(
      target, triple, option);
  if (machine == NULL)
    goto except_machine;

  LLVMTargetDataRef layout;
  if ((layout = LLVMCreateTargetDataLayout(machine)) == NULL)
    goto except_layout;

  LLVMTypeRef bool_type;
  if ((bool_type = LLVMInt1Type()) == NULL)
    goto except_bool_type;

  LLVMTypeRef byte_type;
  if ((byte_type = LLVMInt8Type()) == NULL)
    goto except_byte_type;

  LLVMTypeRef size_type;
  if ((size_type = LLVMIntPtrType(layout)) == NULL)
    goto except_size_type;

  LLVMTypeRef star_type;
  if ((star_type = LLVMPointerType(byte_type, 0)) == NULL)
    goto except_star_type;

  LLVMTypeRef vector_argv[] = { size_type, star_type };
  LLVMTypeRef vector_type;
  if ((vector_type = LLVMStructType(vector_argv, 2, 0)) == NULL)
    return NULL;

  LLVMTypeRef malloc_argv[] = { size_type };
  LLVMTypeRef malloc_type;
  if ((malloc_type = LLVMFunctionType(star_type, malloc_argv, 1, 0)) == NULL)
    goto except_malloc_type;

  LLVMModuleRef module;
  if ((module = LLVMModuleCreateWithName("test_module")) == NULL)
    goto except_module;
  LLVMSetTarget(module, triple);
  LLVMSetModuleDataLayout(module, layout);

  LLVMValueRef malloc;
  if ((malloc = LLVMAddFunction(module, "malloc", malloc_type)) == NULL)
    goto except_malloc;
  LLVMSetLinkage(malloc, LLVMExternalLinkage);

  LLVMValueRef zero_size = LLVMConstInt(size_type, 0, 0);

  *author = (author_t) {
    .detect = detect,
    .inductor = inductor,
    .type_length = inductor->type_number,
    .node_length = as_engine(inductor->engine)->node_number,
    .layout = layout,
    .module = module,
    .bool_type = bool_type,
    .byte_type = byte_type,
    .size_type = size_type,
    .star_type = star_type,
    .vector_type = vector_type,
    .malloc_type = malloc_type,
    .malloc = malloc,
    .zero_size = zero_size,
  };
  return author;

except_malloc:
  LLVMDisposeModule(module);

except_module:

except_malloc_type:
except_star_type:
except_size_type:
except_byte_type:
except_bool_type:
  LLVMDisposeTargetData(layout);

except_layout:
  LLVMDisposeTargetMachine(machine);

except_machine:
  LLVMDisposeMessage(features);

except_features:
  LLVMDisposeMessage(cpu);

except_cpu:
  LLVMDisposeTargetMachineOptions(option);

except_option:
except_target:
  LLVMDisposeMessage(triple);

except_triple:
  return NULL;
}
