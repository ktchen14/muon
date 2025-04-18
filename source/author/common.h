#include "../inductor.h"

#include <llvm-c/Target.h>
#include <llvm-c/Types.h>

#include <stddef.h>

typedef struct emitter_t emitter_t;

typedef struct {
  emitter_t *emitter;
  LLVMValueRef lambda;
  LLVMBuilderRef builder;
} frame_t;

struct emitter_t {
  detect_result_t *detect;
  mu_inductor_t *inductor;

  LLVMTypeRef type_to_type[1000];
  size_t type_length;

  LLVMValueRef node_to_value[1000];
  size_t node_length;

  frame_t frame[1000];
  size_t frame_length;

  LLVMModuleRef module;
  LLVMTargetDataRef data_layout;
};

LLVMTypeRef get_type(emitter_t *emitter, const mu_type_t *root);
