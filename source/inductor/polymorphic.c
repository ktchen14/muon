void mark_type(induce_t *induce, const mu_type_t *type, _Bool negative, size_t rank) {
  switch (type->kind) {
    case MU_CORE_TYPE: {
      const mu_core_type_t *core_type = (const mu_core_type_t *) type;
      const mu_core_t *core = core_type->core;

      for (size_t i = 0; i < core_type->core->argc; i++) {
        mu_variance_t variance = core->argv[i].variance;
        assert(variance != MU_INVARIANCE);
        negative = variance == MU_COVARIANCE ? negative : !negative;
        mark_type(induce, core_type->argv[i], negative, rank);
      }

      break;
    }

    case MU_VARIABLE_TYPE: {
      const mu_variable_type_t *variable_type = (const mu_variable_type_t *) type;

      if (variable_type->rank < rank)
        return;

      if (!negative) {
        ((mu_variable_type_t *) variable_type)->positively_reachable = 1;

        for (size_t i = 0; i < induce->edge_length; i++) {
          induce_edge_t sub = induce->edge[i];
          if (sub.upper != &variable_type->as_type)
            continue;
          mark_type(induce, sub.lower, negative, rank);
        }
      } else {
        ((mu_variable_type_t *) variable_type)->negatively_reachable = 1;

        for (size_t i = 0; i < induce->edge_length; i++) {
          induce_edge_t sub = induce->edge[i];
          if (sub.lower != &variable_type->as_type)
            continue;
          mark_type(induce, sub.upper, negative, rank);
        }
      }
      break;
    }

    case MU_SCHEME_TYPE:
      abort();
  }
}


typedef struct {
  const mu_type_t *source;
  const mu_type_t *target;
} cache_item;

/* const type_t *instantiate_single_type( */
/*     induce_t *induce, */
/*     const type_t *type, */
/*     const type_t *scheme, */
/*     open_scheme_t *target_scheme, */
/*     cache_item *cache, */
/*     size_t *cache_i */
/* ) { */
/*   for (size_t i = 0; i < 100; i++) { */
/*     if (cache[i].source == type) */
/*       return cache[i].target; */
/*   } */

/*   switch (type->kind) { */
/*     case SIMPLE_TYPE: */
/*       switch (type->core->kind) { */
/*         case MU_BOOLEAN_CORE: */
/*         case MU_INTEGER_CORE: */
/*           cache[(*cache_i)++] = (cache_item) { type, type }; */
/*           return type; */

/*         case MU_LAMBDA_CORE: */
/*         { */
/*           const type_t *argv_0 = instantiate_single_type(induce, type->argv[0], scheme, target_scheme, cache, cache_i); */
/*           const type_t *argv_1 = instantiate_single_type(induce, type->argv[1], scheme, target_scheme, cache, cache_i); */

/*           const type_t *result = type; */
/*           if (argv_0 != type->argv[0] || argv_1 == type->argv[1]) */
/*             result = lambda_type(induce, argv_0, argv_1); */
/*           cache[(*cache_i)++] = (cache_item) { type, result }; */
/*           return result; */
/*         } */

/*         case MU_VECTOR_CORE: */
/*         { */
/*           const type_t *argv_0 = instantiate_single_type(induce, type->argv[0], scheme, target_scheme, cache, cache_i); */
/*           const type_t *result = type; */
/*           if (argv_0 != type->argv[0]) */
/*             result = vector_type(induce, argv_0); */
/*           cache[(*cache_i)++] = (cache_item) { type, result }; */
/*           return result; */
/*         } */
/*       } */
/*       break; */

/*     case RECORD_TYPE: */
/*     { */
/*       type_t *mut; */
/*       if ((mut = record_type_allocate(induce, type->argc)) == NULL) */
/*         return NULL; */

/*       _Bool is_same = 1; */
/*       for (size_t i = 0; i < type->argc; i++) { */
/*         mut->schema[i].name = type->schema[i].name; */
/*         mut->schema[i].type = instantiate_single_type(induce, type->schema[i].type, scheme, target_scheme, cache, cache_i); */
/*         if (mut->schema[i].type != type->schema[i].type) */
/*           is_same = 0; */
/*       } */

/*       const type_t *result = type; */
/*       if (!is_same) */
/*         result = record_type_activate(mut); */
/*       cache[(*cache_i)++] = (cache_item) { type, result }; */
/*       return result; */
/*     } */

/*     case VARIABLE_TYPE: */
/*     { */
/*       if (type->polymorphic_to != scheme) */
/*         return type; */

/*       const type_t *newvar; */
/*       if ((newvar = variable_type(induce, target_scheme)) == NULL) */
/*         return NULL; */
/*       cache[(*cache_i)++] = (cache_item) { type, newvar }; */

/*       for (size_t i = 0; i < induce->sub_length; i++) { */
/*         const induce_sub_t sub = induce->sub_data[i]; */
/*         if (sub.lower == type) */
/*           append(induce, newvar, instantiate_single_type(induce, sub.upper, scheme, target_scheme, cache, cache_i)); */
/*         if (sub.upper == type) */
/*           append(induce, instantiate_single_type(induce, sub.lower, scheme, target_scheme, cache, cache_i), newvar); */
/*       } */

/*       return newvar; */
/*     } */

/*     case SCHEME_TYPE: */
/*       fprintf(stderr, "Unsupported higher rank polymorphism\n"); */
/*       abort(); */

/*     case JOIN_TYPE: */
/*     { */
/*       type_t *mut; */
/*       if ((mut = join_type_allocate(induce, type->join_argc)) == NULL) */
/*         return NULL; */

/*       _Bool is_same = 1; */
/*       for (size_t i = 0; i < type->argc; i++) { */
/*         mut->join_argv[i] = instantiate_single_type(induce, type->join_argv[i], scheme, target_scheme, cache, cache_i); */
/*         if (mut->join_argv[i] != type->join_argv[i]) */
/*           is_same = 0; */
/*       } */

/*       const type_t *result = type; */
/*       if (!is_same) */
/*         result = join_type_activate(mut); */
/*       cache[(*cache_i)++] = (cache_item) { type, result }; */
/*       return result; */
/*     } */
/*   } */
/* } */

/* const type_t *instantiate_scheme( */
/*     induce_t *induce, const type_t *type, open_scheme_t *target_scheme */
/* ) { */
/*   assert(type->kind == SCHEME_TYPE); */
/*   cache_item cache[100] = {0}; */
/*   size_t i = 0; */
/*   return instantiate_single_type(induce, type->matter, type, target_scheme, cache, &i); */
/* } */

