#ifndef MU_DETECTOR_DETECT_I
#define MU_DETECTOR_DETECT_I

#include "../engine.h"
#include "../status.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
  const MuonEngine *engine;
  size_t length;
  MuonNode *data[/* length */];
} detect_result_t;

typedef struct {
  mu_status_t *status;
  detect_result_t *result;
  const MuonModule *module;
} detect_t;

MUON_HINT(nonnull, pure)
static inline MuonNode *detect_evince(
    const detect_result_t *detect, MuonNode *node) {
  assert(node->id < detect->length);
  MuonNode *result = detect->data[node->id];
  assert(result != NULL);
  return result;
}

MUON_HINT(nonnull, pure)
static inline MuonNode *detect_evince_loose(
    const detect_result_t *detect, MuonNode *node) {
  assert(node->id < detect->length);
  return detect->data[node->id];
}

MUON_HINT(nonnull, pure)
static inline const detect_result_t *detect_result(const detect_t *detect) {
  return detect->result;
}

detect_t *detect_initialize(
    detect_t *detect,
    const MuonEngine *engine,
    mu_status_t *status,
    const MuonModule *module)
  MUON_HINT_SUFFIX(nonnull);

detect_t *detect_node(detect_t *detect, MuonNode *node);

#endif /* MU_DETECTOR_DETECT_I */
