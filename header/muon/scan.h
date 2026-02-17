#ifndef MUON_SCAN_H
#define MUON_SCAN_H

#include "engine.h"
#include "status.h"

MuonScript *muon_scan(MuonEngine *engine, mu_status_t *status, const char *text)
  MUON_HINT_SUFFIX(nonnull);

#endif /* MUON_SCAN_H */
