#include "standard.h"

#include "engine.h"

#include <string.h>

MuonModule *muon_standard_module(MuonEngine *engine) {
  MuonCoreType *boolean_type;
  if ((boolean_type = muon_boolean_type(engine)) == NULL)
    return NULL;

  MuonCoreType *integer_type;
  if ((integer_type = muon_integer_type(engine)) == NULL)
    return NULL;

  MuonType *argv[] = {&integer_type->as_type, &boolean_type->as_type};
  MuonCoreType *lambda_type;
  if ((lambda_type = muon_lambda_type(engine, argv[0], argv[1])) == NULL)
    return NULL;

  MuonName *name;
  if ((name = muon_name(engine, strlen("is_nonzero"), "is_nonzero")) == NULL)
    return NULL;

  MuonExport *is_nonzero;
  if ((is_nonzero = muon_export(engine, name, &lambda_type->as_type)) == NULL)
    return NULL;

  MuonModule *result;
  if ((result = muon_module(engine, 1, (MuonExport *[]) {is_nonzero})) == NULL)
    return NULL;
  return result;
}
