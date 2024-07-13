#include "status.h"

#include "stator.h"

const mu_memo_t memo = {0};

const mu_memo_t *mu_memo(
    mu_status_t *status,
    const mu_node_source_t *source,
    const char *restrict format,
    ...) {
  return &memo;
}
