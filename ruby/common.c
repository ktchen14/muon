#include "common.h"
#include "node.h"

#include <muon.h>
#include <stddef.h>

static size_t engine_size(const void *data) {
  return sizeof(MuonEngine);
}

const rb_data_type_t muon_engine_type = {
  .wrap_struct_name = "Muon::Engine",
  .function = {.dfree = RUBY_DEFAULT_FREE, .dsize = engine_size},
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE engine_alloc(VALUE klass) {
  MuonEngine *engine = calloc(1, sizeof(MuonEngine));
  if (engine == NULL)
    rb_raise(rb_eNoMemError, "failed to allocate MuonEngine");

  VALUE self = TypedData_Wrap_Struct(klass, &muon_engine_type, engine);
  engine->remote = (void *) self;
  return self;
}

static VALUE engine_initialize(VALUE self) {
  return self;
}

// Engine#scan(text)
static VALUE engine_scan(VALUE self, VALUE rb_text) {
  MuonEngine *engine = as_muon_engine(self);
  const char *text = StringValueCStr(rb_text);
  char status[sizeof(void *)] = {0};
  MuonScript *script;
  if ((script = muon_scan(engine, (mu_status_t *) status, text)) == NULL)
    rb_raise(rb_eRuntimeError, "Failed to scan");
  return as_script(script);
}

void Init_muon_engine(VALUE mMuon) {
  VALUE cEngine = rb_define_class_under(mMuon, "Engine", rb_cObject);
  rb_define_alloc_func(cEngine, engine_alloc);
  rb_define_method(cEngine, "initialize", engine_initialize, 0);
  rb_define_method(cEngine, "scan", engine_scan, 1);
}
