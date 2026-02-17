all:
	$(MAKE) -C build $@

.PHONY: test
test:
	$(MAKE) -C build test

.PHONY: format
format:
	scripts/muon-format -i header/muon.h
	scripts/muon-format -i header/muon/common.h
	scripts/muon-format -i header/muon/engine.h
	scripts/muon-format -i header/muon/engine/common.h
	scripts/muon-format -i header/muon/engine/name.h
	scripts/muon-format -i header/muon/engine/node.h
	scripts/muon-format -i header/muon/inductor.h
	scripts/muon-format -i header/muon/inductor/core.h
	scripts/muon-format -i header/muon/inductor/coercion.h
	scripts/muon-format -i header/muon/inductor/type.h
	scripts/muon-format -i header/muon/scan.h
	scripts/muon-format -i header/muon/status.h

%:
	$(MAKE) -C build $@
