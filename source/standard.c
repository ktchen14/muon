#include "standard.h"

#include "engine.h"

#include <string.h>

static MuonName *muon_nominate(MuonEngine *engine, const char *string) {
  return muon_name(engine, strlen(string), string);
}

MuonModule *muon_standard_module(MuonEngine *engine) {
  MuonExport *is_integer = muon_export(
      engine,
      muon_nominate(engine, "is_integer"),
      &muon_lambda_type(
          engine,
          &muon_integer_type(engine)->as_type,
          &muon_boolean_type(engine)->as_type)
          ->as_type);

  MuonExport *is_integer_list = muon_export(
      engine,
      muon_nominate(engine, "is_integer_list"),
      &muon_lambda_type(
          engine,
          &muon_vector_type(engine, &muon_integer_type(engine)->as_type)->as_type,
          &muon_boolean_type(engine)->as_type)
          ->as_type);

  MuonExport *is_boolean = muon_export(
      engine,
      muon_nominate(engine, "is_boolean"),
      &muon_lambda_type(
          engine,
          &muon_boolean_type(engine)->as_type,
          &muon_boolean_type(engine)->as_type)
          ->as_type);

  MuonModule *result;
  if ((result = muon_module(
           engine, 3, (MuonExport *[]) {is_integer, is_integer_list, is_boolean}))
      == NULL)
    return NULL;
  return result;
}
