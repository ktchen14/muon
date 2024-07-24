#include "lambda_type.h"

#include "engine.h"
#include "type.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const mu_lambda_type_t *mu_lambda_type(
    mu_engine_t *engine, const mu_type_t *argument, const mu_type_t *output) {
  assert(argument->as_stator.engine == engine);
  assert(output->as_stator.engine == engine);

  size_t size = sizeof(mu_lambda_type_t);

  mu_lambda_type_t *result;
  if ((result = type_allocate(engine, size)) == NULL)
    return NULL;
  *result = (mu_lambda_type_t) {
    .as_type.kind = MU_LAMBDA_TYPE, .argument = argument, .output = output,
  };

  return assign_type(engine, result);
}

const mu_lambda_type_t *lambda_type_import(
    const mu_lambda_type_t *type, const import_t *import) {
  const mu_type_t *argument = import_retrieve(import->type, type->argument);
  const mu_type_t *output = import_retrieve(import->type, type->output);
  if (argument == type->argument && output == type->output)
    return type;
  return mu_lambda_type(import->engine, argument, output);
}

#include "debug.h"

void mu_lambda_type_debug(const mu_lambda_type_t *type) {
  WITH_DEBUG_NEGATE() { mu_type_debug(type->argument); }
  fputs(" → ", stderr);
  mu_type_debug(type->output);
}
