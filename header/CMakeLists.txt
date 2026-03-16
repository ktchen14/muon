target_sources(muon-header INTERFACE FILE_SET HEADERS FILES
  muon/common.h

  muon/engine.h
  muon/engine/common.h
  muon/engine/core.h
  muon/engine/instance.h
  muon/engine/module.h
  muon/engine/name.h
  muon/engine/node.h
  muon/engine/stator.h
  muon/engine/type.h

  muon/inductor/coercion.h
  muon/inductor.h

  muon/scan.h
  muon/status.h
  muon.h
)
