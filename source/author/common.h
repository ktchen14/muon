#include "../inductor.h"
#include "../stator.h"

#include <llvm-c/Target.h>
#include <llvm-c/Types.h>

#include <stddef.h>

/**
 * @brief An internal structure used by an author to keep track of where to
 * write instructions to
 */
typedef struct {
  LLVMValueRef lambda;
  LLVMBuilderRef tail;
} stream_t;

typedef struct {
  const detect_result_t *detect;
  mu_inductor_t *inductor;

  LLVMTypeRef type_to_type[1000];
  size_t type_length;

  LLVMValueRef node_to_value[1000];
  size_t node_length;

  /// LLVM data layout
  LLVMTargetDataRef layout;

  /// LLVM type of a Boolean
  LLVMTypeRef bool_type;

  /// LLVM type of a byte
  LLVMTypeRef byte_type;

  /// LLVM type of a size_t
  LLVMTypeRef size_type;

  /// LLVM type of an opaque pointer
  LLVMTypeRef star_type;

  /// LLVM type of a Muon vector
  LLVMTypeRef vector_type;

  /// LLVM type of malloc()
  LLVMTypeRef malloc_type;

  /// The active module
  LLVMModuleRef module;

  /// External declaration of malloc() in the module
  LLVMValueRef malloc;

  stream_t stream[1000];
  size_t stream_length;

  /// The active lambda
  LLVMValueRef lambda;

  /// Builder positioned at the end of the lambda
  LLVMBuilderRef tail;
} author_t;

typedef struct {
  const mu_type_t *source_muon_type;
  LLVMTypeRef source_type;
  LLVMValueRef source;
} info_t;

author_t *author_initialize(
    author_t *author, const detect_result_t *detect, mu_inductor_t *inductor)
  __attribute__((nonnull));

LLVMTypeRef get_type(author_t *author, const mu_type_t *root);
LLVMModuleRef script_emit(induce_t *induce, const mu_node_t *root);
LLVMValueRef coercion_emit(author_t *author, const mu_coercion_t *coercion, info_t info);

static inline LLVMValueRef author_continue(
    author_t *author, LLVMValueRef lambda, LLVMBuilderRef tail) {
  author->stream[author->stream_length++] = (stream_t) {
    .lambda = author->lambda, .tail = author->tail,
  };
  author->lambda = lambda;
  author->tail = tail;
  return lambda;
}

static inline LLVMValueRef author_return(author_t *author) {
  LLVMValueRef result = author->lambda;

  stream_t stream = author->stream[--author->stream_length];
  author->lambda = stream.lambda;
  author->tail = stream.tail;
  return result;
}
