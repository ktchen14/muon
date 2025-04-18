#include "../inductor.h"

#include <llvm-c/Target.h>
#include <llvm-c/Types.h>

#include <stddef.h>

typedef struct author_t author_t;

typedef struct {
  author_t *author;
  LLVMValueRef lambda;
  LLVMBuilderRef builder;
} frame_t;

struct author_t {
  const detect_result_t *detect;
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

LLVMTypeRef get_type(author_t *author, const mu_type_t *root);
