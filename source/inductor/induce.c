#include "induce.h"

#include "common.h"
#include "scheme.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static Rule *retrieve_core_coercion(
    Inductor *inductor, MuonCoreType *source, MuonCoreType *target, Rule *rule);

Rule *type_restrain(
    Inductor *inductor, MuonType *source, MuonType *target, MuonNode *reason) {
  assert(source != target);

  Rule *result;
  if ((result = rule_search(inductor, source, target)) != NULL)
    return result;

  // A join type shouldn't ever appear as a source, and a meet type shouldn't
  // ever appear as a target.
  assert(target->tag != MUON_JOIN_TYPE);
  assert(source->tag != MUON_MEET_TYPE);

  // A scheme type shouldn't appear as a target unless we're dealing with a
  // higher ranked type, which we don't support at the moment
  assert(target->tag != MUON_SCHEME_TYPE);

  // Make ⟨source ⇒ target⟩ here in case of recursion
  if ((result = rule_insert(inductor, source, target)) == NULL)
    return NULL;
  result->reason = reason;

  if (is_implicit_type(source) && !is_implicit_type(target)) {
    // ∀(τ) | ∃⟨τ ⇒ source⟩ and τ isn't a implicit type, restrain τ ⇒ target
    RuleIterator it = rule_iterator(inductor, (Attitude) {source, 0});
    for (const Rule *rule; (rule = rule_next(&it)) != NULL;) {
      if (is_implicit_type(rule->source))
        continue;

      if (type_restrain(inductor, rule->source, target, reason) == NULL)
        return NULL;
    }

    // ∀(τ) | ∃⟨τ ⇒ source⟩ and τ is a implicit type, create ⟨τ ⇒ target⟩ to
    // maintain the target-side transitive closure of τ
    it = rule_iterator(inductor, (Attitude) {source, 0});
    for (const Rule *rule; (rule = rule_next(&it)) != NULL;) {
      if (!is_implicit_type(rule->source))
        continue;

      Rule *next;
      if ((next = rule_search(inductor, rule->source, target)) != NULL)
        continue;

      if ((next = rule_insert(inductor, rule->source, target)) == NULL)
        return NULL;
      next->tag = INDIRECT_RULE;
      next->center = source;
      next->reason = reason;
    }

    return result;
  }

  if (!is_implicit_type(source) && is_implicit_type(target)) {
    // ∀(τ) | ∃⟨target ⇒ τ⟩ and τ isn't a implicit type, restrain source ⇒ τ
    RuleIterator it = rule_iterator(inductor, (Attitude) {target, 1});
    for (const Rule *rule; (rule = rule_next(&it)) != NULL;) {
      if (is_implicit_type(rule->target))
        continue;

      if (type_restrain(inductor, source, rule->target, reason) == NULL)
        return NULL;
    }

    // ∀(τ) | ∃⟨target ⇒ τ⟩ and τ is a implicit type, create ⟨source ⇒ τ⟩ to
    // maintain the source-side transitive closure of τ
    it = rule_iterator(inductor, (Attitude) {target, 1});
    for (const Rule *rule; (rule = rule_next(&it)) != NULL;) {
      if (!is_implicit_type(rule->target))
        continue;

      Rule *next;
      if ((next = rule_search(inductor, source, rule->target)) != NULL)
        continue;

      if ((next = rule_insert(inductor, source, rule->target)) == NULL)
        return NULL;
      next->tag = INDIRECT_RULE;
      next->center = target;
      next->reason = reason;
    }

    return result;
  }

  if (is_implicit_type(source) && is_implicit_type(target)) {
    RuleIterator it, jt;

    // ∀(α, β) | ∃⟨α ⇒ source⟩ ∧ ∃⟨target ⇒ β⟩ and α and β aren't variable
    // types, restrain α ⇒ β
    it = rule_iterator(inductor, (Attitude) {source, 0});
    for (const Rule *a_rule; (a_rule = rule_next(&it)) != NULL;) {
      if (is_implicit_type(a_rule->source))
        continue;

      RuleIterator jt = rule_iterator(inductor, (Attitude) {target, 1});
      for (const Rule *b_rule; (b_rule = rule_next(&jt)) != NULL;) {
        if (is_implicit_type(b_rule->target))
          continue;

        if (type_restrain(inductor, a_rule->source, b_rule->target, reason)
            == NULL)
          return NULL;
      }
    }

    // ∀(α) | ∃⟨α ⇒ source⟩, create ⟨α ⇒ target⟩ to maintain the target-side
    // transitive closure of α
    it = rule_iterator(inductor, (Attitude) {source, 0});
    for (const Rule *a_rule; (a_rule = rule_next(&it)) != NULL;) {
      Rule *next;
      if ((next = rule_search(inductor, a_rule->source, target)) != NULL)
        continue;

      if ((next = rule_insert(inductor, a_rule->source, target)) == NULL)
        return NULL;
      next->tag = INDIRECT_RULE;
      next->center = source;
      next->reason = reason;

      RuleIterator jt = rule_iterator(inductor, (Attitude) {target, 1});
      for (const Rule *b_rule; (b_rule = rule_next(&jt)) != NULL;) {
        if (!is_implicit_type(b_rule->target))
          continue;

        Rule *next;
        if ((next = rule_search(inductor, a_rule->source, b_rule->target))
            != NULL)
          continue;

        if ((next = rule_insert(inductor, a_rule->source, b_rule->target))
            == NULL)
          return NULL;
        next->tag = INDIRECT_RULE;
        next->center = source;
        next->reason = reason;
      }
    }

    // ∀(β) | ∃⟨target ⇒ β⟩, create ⟨source ⇒ β⟩ to maintain the source-side
    // transitive closure of β
    jt = rule_iterator(inductor, (Attitude) {target, 1});
    for (const Rule *b_rule; (b_rule = rule_next(&jt)) != NULL;) {
      Rule *next;
      if ((next = rule_search(inductor, source, b_rule->target)) != NULL)
        continue;

      if ((next = rule_insert(inductor, source, b_rule->target)) == NULL)
        return NULL;
      next->tag = INDIRECT_RULE;
      next->center = target;
      next->reason = reason;

      RuleIterator jt = rule_iterator(inductor, (Attitude) {source, 0});
      for (const Rule *a_rule; (a_rule = rule_next(&jt)) != NULL;) {
        if (!is_implicit_type(a_rule->source))
          continue;

        Rule *next;
        if ((next = rule_search(inductor, a_rule->source, b_rule->target))
            != NULL)
          continue;

        if ((next = rule_insert(inductor, a_rule->source, b_rule->target))
            == NULL)
          return NULL;
        next->tag = INDIRECT_RULE;
        next->center = source;
        next->reason = reason;
      }
    }

    return result;
  }

  MuonJoinType *join_type;
  if ((join_type = muon_type_cast(source, join_type)) != NULL) {
    for (size_t i = 0; i < join_type->argc; i++) {
      MuonType *argument = join_type->argv[i];
      if (type_restrain(inductor, argument, target, reason) == NULL)
        return NULL;
    }
    return result;
  }

  MuonMeetType *meet_type;
  if ((meet_type = muon_type_cast(target, meet_type)) != NULL) {
    for (size_t i = 0; i < meet_type->argc; i++) {
      MuonType *argument = meet_type->argv[i];
      if (type_restrain(inductor, source, argument, reason) == NULL)
        return NULL;
    }
    return result;
  }

  MuonSchemeType *scheme_type;
  if ((scheme_type = muon_type_cast(source, scheme_type)) != NULL) {
    MuonType *instance;
    if ((instance = scheme_instance(inductor, scheme_type)) == NULL)
      return NULL;

    if (type_restrain(inductor, instance, target, reason) == NULL)
      return NULL;
    return result;
  }

  MuonCoreType *core_source = muon_type_cast(source, core_source);
  assert(core_source != NULL);

  MuonCoreType *core_target = muon_type_cast(target, core_target);
  assert(core_target != NULL);

  if (core_source->core != core_target->core) {
    fprintf(stderr, "Type mismatch. Expected ");
    muon_core_debug(core_target->core);
    fprintf(stderr, " but got ");
    muon_core_debug(core_source->core);
    fprintf(stderr, "\n");
    abort();
  }

  MuonCore *core = core_source->core;
  for (size_t i = 0; i < core_argc(core); i++) {
    MuonType *next_source = core_source->argv[i];
    MuonType *next_target = core_target->argv[i];

    _Bool variance = core_at(core, i).variance;
    if (variance) {
      MuonType *t = next_source;
      next_source = next_target;
      next_target = t;
    }

    if (type_restrain(inductor, next_source, next_target, reason) == NULL)
      return NULL;
  }

  return result;
}

Rule *type_assess(
    Inductor *inductor, MuonType *restrict source, MuonType *restrict target) {
  // If ∃⟨source ⇒ target⟩ then return the coercion on that edge
  Rule *result;
  if ((result = rule_search(inductor, source, target)) != NULL)
    return result;

  if (source->tag == MUON_SCHEME_TYPE || target->tag == MUON_SCHEME_TYPE) {
    // If ∃⟨source ⇒ target⟩ then return the coercion on that edge
    Rule *result;
    if ((result = rule_search(inductor, source, target)) != NULL)
      return result;

    // Make ⟨source ⇒ target⟩ here in case of recursion
    if ((result = rule_insert(inductor, source, target)) == NULL)
      return NULL;
    result->tag = REJECTED_RULE;
    return result;
  }
  assert(source->tag != MUON_SCHEME_TYPE && target->tag != MUON_SCHEME_TYPE);

  // Make ⟨source ⇒ target⟩ here in case of recursion
  if ((result = rule_insert(inductor, source, target)) == NULL)
    return NULL;

  if (source->tag == MUON_CORE_TYPE && target->tag == MUON_CORE_TYPE) {
    MuonCoreType *core_source = (MuonCoreType *) source;
    MuonCoreType *core_target = (MuonCoreType *) target;

    if ((result = retrieve_core_coercion(
             inductor, core_source, core_target, result))
        == NULL)
      return NULL;
    return result;
  }

  result->tag = REJECTED_RULE;
  return result;
}

static Rule *retrieve_core_coercion(
    Inductor *inductor,
    MuonCoreType *source,
    MuonCoreType *target,
    Rule *rule) {
  /* if (source_core->kind == MUON_INTEGER_CORE && target_core->kind ==
   * MUON_RECORD_CORE) */
  /*   return induce->id_coercion; */

  /* if (source_core->kind == MUON_BOOLEAN_CORE && target_core->kind ==
   * MUON_INTEGER_CORE) */
  /*   return induce->id_coercion; */

  if (source->core != target->core)
    return rule->tag = REJECTED_RULE, rule;

  MuonCore *core = source->core;

  for (size_t i = 0; i < core_argc(core); i++) {
    MuonCoreMember member = core_at(core, i);

    MuonType *next_source = source->argv[member.i];
    MuonType *next_target = target->argv[member.i];

    if (member.variance) {
      MuonType *t;
      t = next_source;
      next_source = next_target;
      next_target = t;
    }

    Rule *next;
    if ((next = type_assess(inductor, next_source, next_target)) == NULL)
      return NULL;
    if (next->tag == REJECTED_RULE) {
      rule->tag = REJECTED_RULE;
      break;
    }
  }

  return rule;
}
