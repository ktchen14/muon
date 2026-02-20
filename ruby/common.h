#ifndef MUON_RUBY_COMMON_H
#define MUON_RUBY_COMMON_H

#include <muon.h>
#include <ruby.h>
#include <string.h>

extern const rb_data_type_t muon_engine_type;

static inline MuonEngine *as_muon_engine(VALUE rb_engine) {
  MuonEngine *engine;
  TypedData_Get_Struct(rb_engine, MuonEngine, &muon_engine_type, engine);
  return engine;
}

static inline MuonName *as_muon_name(MuonEngine *engine, VALUE rb_name) {
  const char *text = rb_id2name(rb_sym2id(rb_name));
  MuonName *name;
  if ((name = muon_name(engine, strlen(text), text)) == NULL)
    rb_raise(rb_eNoMemError, "Failed to allocate MuonName");
  return name;
}

static inline VALUE as_symbol(MuonName *name) {
  return ID2SYM(rb_intern2(name->text, (long) name->length));
}

/// Initialize the Engine class under the given module
void Init_muon_engine(VALUE mMuon);

#endif /* MUON_RUBY_COMMON_H */
