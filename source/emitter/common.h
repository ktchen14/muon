#include "../inductor.h"

#include <llvm-c/Target.h>
#include <llvm-c/Types.h>

#include <stddef.h>

typedef struct emit_t emit_t;

typedef struct {
  emit_t *emit;
  LLVMValueRef lambda;
  LLVMBuilderRef builder;
} frame_t;

struct emit_t {
  LLVMModuleRef module;
  detect_result_t *detect;
  frame_t frame[1000];
  size_t frame_length;
};

typedef struct {
  LLVMTypeRef type_to_type[1000];
  size_t type_length;
  LLVMTargetDataRef data_layout;
} emitter_t;
