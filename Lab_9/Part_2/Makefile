.PHONY: all build prog clean

IPECMD ?= ipecmd
HEX ?= $(abspath build/Lab9_Part2_app.hex)

all: build
	$(MAKE) prog

build:
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$(abspath ../../PIC18F46K22.cmake)
	cmake --build build --parallel

prog:
	@test -f "$(HEX)" || (echo "Hex file not found: $(HEX). Run make build first." >&2; exit 1)
	$(IPECMD) -P18F46K22 -TPPKOB4 -OK -OL -M -F"$(HEX)"

clean:
	rm -rf build
