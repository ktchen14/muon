target_sources(muon-common INTERFACE FILE_SET HEADERS FILES
  muon/common.h

  muon/engine/common.h
  muon/engine/core.h
  muon/engine/name.h
  muon/engine/node.h
  muon/engine/stator.h
  muon/engine/type.h
  muon/engine.h

  muon/inductor/common.h
  muon/inductor/coercion.h
  muon/inductor.h

  muon/scan.h
  muon/status.h
  muon.h
)
