.PHONY: all clean build prog

IPECMD ?= ipecmd
HEX ?= $(abspath $(firstword $(wildcard build/*.hex)))

all: build prog

build:
	mkdir -p build
	cd build && cmake -DCMAKE_TOOLCHAIN_FILE=../PIC18F46K22.cmake ../ && $(MAKE) -j$$(nproc)

clean: 
	rm -rf build

prog:
	@test -n "$(HEX)" || (echo "No hex file found in build/. Pass one with: make prog HEX=/path/to/file.hex" >&2; exit 1)
	@test -f "$(HEX)" || (echo "Hex file not found: $(HEX)" >&2; exit 1)
	$(IPECMD) -P18F46K22 -TPPKOB4 -OK -OL -M -F"$(HEX)"
