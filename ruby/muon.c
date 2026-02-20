#include <muon.h>

#include <ruby.h>

static void engine_free(void *data) {
  free(data);
}

static size_t engine_memsize(const void *data) {
  return sizeof(MuonEngine);
}

static const rb_data_type_t muon_engine_type = {
  .wrap_struct_name = "Muon::Engine",
  .function = {
    .dmark = NULL,
    .dfree = engine_free,
    .dsize = engine_memsize,
  },
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE engine_alloc(VALUE klass) {
  MuonEngine *engine = calloc(1, sizeof(MuonEngine));
  if (engine == NULL)
    rb_raise(rb_eNoMemError, "failed to allocate MuonEngine");

  return TypedData_Wrap_Struct(klass, &muon_engine_type, engine);
}

static VALUE engine_initialize(VALUE self) {
  return self;
}

void Init_muon(void) {
  VALUE mMuon = rb_define_module("Muon");

  VALUE cEngine = rb_define_class_under(mMuon, "Engine", rb_cObject);
  rb_define_alloc_func(cEngine, engine_alloc);
  rb_define_method(cEngine, "initialize", engine_initialize, 0);
}
