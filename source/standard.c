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
          &muon_vector_type(engine, &muon_integer_type(engine)->as_type)
              ->as_type,
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

  MuonType *bottom_type = &as_engine(engine)->bottom_type->as_type;
  MuonType *object_type = &as_engine(engine)->object_type->as_type;

  struct MuonSchemeType *scheme_allocation = scheme_type_allocate(engine, 1);

  MuonVariableType *variable_type = muon_variable_type(
      engine,
      scheme_allocation,
      muon_nominate(engine, "t"),
      bottom_type,
      object_type);

  scheme_allocation = scheme_type_initiate(scheme_allocation);

  scheme_allocation->matter = &muon_lambda_type(
      engine,
      &muon_vector_type(engine, &variable_type->as_type)->as_type,
      &variable_type->as_type)
                             ->as_type;

  scheme_allocation->argv[0] = variable_type;
  MuonSchemeType *scheme_type = scheme_type_activate(engine);

  MuonExport *list_head = muon_export(
      engine, muon_nominate(engine, "list_head"), &scheme_type->as_type);

  MuonModule *result;
  if ((result = muon_module(
           engine,
           4,
           (MuonExport *[]) {
             is_integer, is_integer_list, is_boolean, list_head
           }))
      == NULL)
    return NULL;
  return result;
}
