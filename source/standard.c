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
  if ((name = muon_name(engine, strlen("is_integer"), "is_integer")) == NULL)
    return NULL;

  MuonExport *is_integer;
  if ((is_integer = muon_export(engine, name, &lambda_type->as_type)) == NULL)
    return NULL;

  MuonExport *is_boolean;
  is_boolean = muon_export(
      engine,
      muon_name(engine, strlen("is_boolean"), "is_boolean"),
      &muon_lambda_type(engine, &boolean_type->as_type, &boolean_type->as_type)->as_type);

  MuonModule *result;
  if ((result = muon_module(engine, 2, (MuonExport *[]) {is_integer, is_boolean})) == NULL)
    return NULL;
  return result;
}
