#include "../inductor.h"
#include "../stator.h"

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

  LLVMTargetDataRef layout;

  LLVMModuleRef module;

  // LLVM type of a Boolean
  LLVMTypeRef bool_type;

  // LLVM type of a byte
  LLVMTypeRef byte_type;

  // LLVM type of a size_t
  LLVMTypeRef size_type;

  // LLVM type of an opaque pointer
  LLVMTypeRef star_type;

  // LLVM type of a Muon vector
  LLVMTypeRef vector_type;

  // LLVM type of malloc()
  LLVMTypeRef malloc_type;

  LLVMValueRef malloc;
};

typedef struct {
  const mu_type_t *source_muon_type;
  const mu_type_t *target_muon_type;
  LLVMTypeRef source_type;
  LLVMTypeRef target_type;
  LLVMValueRef source;
} info_t;

author_t *author_initialize(
    author_t *author, const detect_result_t *detect, mu_inductor_t *inductor)
  __attribute__((nonnull));

LLVMTypeRef get_type(author_t *author, const mu_type_t *root);
LLVMModuleRef script_emit(induce_t *induce, const mu_node_t *root);
LLVMValueRef coercion_emit(frame_t *frame, const mu_coercion_t *coercion, info_t info);
