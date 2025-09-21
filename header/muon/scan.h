#ifndef MUON_SCAN_H
#define MUON_SCAN_H

#include "engine.h"
#include "status.h"

MuonScript *muon_scan(
    MuonEngine *engine, mu_status_t *status, const char *text)
  __attribute__((nonnull));

#endif /* MUON_SCAN_H */
