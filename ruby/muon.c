#include "common.h"
#include "node.h"

void Init_muon(void) {
  VALUE mMuon = rb_define_module("Muon");

  Init_muon_engine(mMuon);
  Init_muon_node(mMuon);
}
