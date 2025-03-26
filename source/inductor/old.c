  // If source <: x <: target, then:
  //
  //   (source ⇒ target) = (source ⇒ x) ∘ (x ⇒ target)
  //
  // In this case, record ⟨source ⇒ target⟩ as an indirect edge with an indirect
  // coercion through x.
  //
  // This allows us to skip adding source to the lower bound of target at all,
  // so that future calls to ensure_coercion(induce, target, something_else)
  // don't have to check another type, simplifies reduction, etc.
  //
  // We'll skip any subtype x of target that's also a variable type, as well as
  // if ⟨x ⇒ target⟩ is an indirect edge. This is because we don't want the
  // coercion source ⇒ target to occur through some variable type unrelated to
  // the context in which we're ensuring this coercion.
#ifdef OPTIMIZE_EARLY
  iterator = universe_iterator(&induce->universe, target, 0);
  for (const edge_t *edge; (edge = universe_next(&iterator)) != NULL;) {
    if (edge->indirect || edge->source->kind == MU_VARIABLE_TYPE)
      continue;

    // Retrieve the coercion source ⇒ x
    const mu_coercion_t *coercion;
    if ((coercion = retrieve_coercion(induce, source, edge->source)) == NULL)
      return NULL;

    // If we can't make this coercion, then skip this x
    if (coercion == NO_SUCH_COERCION)
      continue;

    // Get the coercion x ⇒ target
    const mu_coercion_t *tail;
    if ((tail = edge_to_coercion(edge)) == NULL)
      return NULL;

    // Create the coercion (source ⇒ x) ∘ (x ⇒ target)
    const mu_indirect_coercion_t *result;
    if ((result = mu_indirect_coercion(coercion, tail)) == NULL)
      return NULL;

    // Make the edge ⟨source ⇒ target⟩
    universe_edge_t *edge;
    if ((edge = append_edge(&induce->universe, source, target)) == NULL)
      return NULL;
    edge->indirect = 2;

    // Record the coercion in the edge
    return edge->coercion = &result->as_coercion;
  }
#endif


  // Once we've committed to ⟨source ⇒ target⟩, try to simplify target:
  //
  //   ∀(next_source) | ∃⟨next_source ⇒ target⟩, next_source ≠ source
  //
  // Retrieve next_source ⇒ source. If this exists, then add
  // ⟨next_source ⇒ source⟩ and make ⟨next_source ⇒ target⟩ an indirect edge.
#ifdef OPTIMIZE_EARLY
  iterator = universe_iterator(&induce->universe, target, 0);
  for (edge_t *edge; (edge = universe_next(&iterator)) != result_edge;) {
    if (edge->indirect || edge->source->kind == MU_VARIABLE_TYPE)
      continue;
    if (edge->coercion != NULL && edge->coercion->kind != MU_EDGE_COERCION)
      continue;

    const mu_coercion_t *coercion;
    if ((coercion = retrieve_coercion(induce, edge->source, source)) == NULL)
      return NULL;

    if (coercion == NO_SUCH_COERCION)
      continue;

    if (edge->coercion == NULL) {
      const mu_indirect_coercion_t *new;
      if ((new = mu_indirect_coercion(coercion, &result->as_coercion)) == NULL)
        return NULL;
      edge->coercion = &new->as_coercion;
    } else {
      mu_indirect_coercion_t new = {
        .as_coercion = { .kind = MU_INDIRECT_COERCION, },
        .head = coercion,
        .tail = &result->as_coercion,
      };

      mu_indirect_coercion_t *over = (mu_indirect_coercion_t *) edge->coercion;
      memcpy(over, &new, sizeof(new));
    }

    edge->indirect = 2;
  }
#endif

