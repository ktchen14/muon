all:
	$(MAKE) -C build $@

.PHONY: test
test:
	$(MAKE) -C build test

.PHONY: format
format:
	clang-format -i header/muon.h
	clang-format -i header/muon/common.h
	clang-format -i header/muon/engine.h
	clang-format -i header/muon/engine/common.h
	clang-format -i header/muon/engine/name.h
	clang-format -i header/muon/engine/node.h
	clang-format -i header/muon/inductor.h
	clang-format -i header/muon/inductor/core.h
	clang-format -i header/muon/inductor/coercion.h
	clang-format -i header/muon/inductor/type.h
	clang-format -i header/muon/scan.h
	clang-format -i header/muon/status.h

%:
	$(MAKE) -C build $@
