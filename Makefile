all:
	$(MAKE) -C build $@

.PHONY: test
test:
	$(MAKE) -C build test

%:
	$(MAKE) -C build $@
