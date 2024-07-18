#ifndef MU_INDUCTOR_DETECT_I
#define MU_INDUCTOR_DETECT_I

#include "../script.h"
#include "../stator.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
  const mu_engine_t *engine;
  size_t length;
  const mu_node_t *data[/* length */];
} detect_result_t;

typedef struct {
  mu_status_t *status;
  detect_result_t *result;
} detect_t;

__attribute__((nonnull, pure))
static inline const mu_node_t *detect_evince(
    const detect_result_t *detect, const mu_node_t *node) {
  assert(node->as_stator.id < detect->length);
  const mu_node_t *result = detect->data[node->as_stator.id];
  assert(result != NULL);
  return result;
}

__attribute__((nonnull, pure))
static inline const mu_node_t *detect_at(
    const detect_result_t *detect, const mu_node_t *node, size_t i) {
  switch (node->kind) {
    case MU_NAME_EXPR_NODE:
    case MU_NAME_SIGN_NODE:
      if ((node = detect_evince(detect, node)) == NULL)
        return NULL;
      return i == 0 ? node : NULL;

    default: return node_at(node, i);
  }
}

detect_t *detect_initialize(
    detect_t *detect, const mu_engine_t *engine, mu_status_t *status)
  __attribute__((nonnull));

detect_t *detect_script(const mu_script_t *script, detect_t *detect);

#endif /* MU_INDUCTOR_DETECT_I */
