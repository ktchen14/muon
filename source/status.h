#ifndef MU_STATUS_I
#define MU_STATUS_I

#include <muon/status.h>

typedef struct memo_t memo_t;

struct memo_t {
  const memo_t *next;

  mu_memo_t _;
};

typedef struct {
  memo_t *tail;
} mu_status_t;

#endif /* MU_STATUS_I */
