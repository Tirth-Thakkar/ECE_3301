# ECE 3301 Lab 8 - standalone XC8 build (alternative to MPLAB X GUI/CMake).
# Requires 'xc8-cc' on PATH; PIC18F46K22 device pack installed for this XC8.
XC8 ?= xc8-cc
MCU ?= 18F46K22
CONF ?= default
OUTDIR := dist/$(CONF)/production
OUT := $(OUTDIR)/Lab8.X.production.hex
ELF := $(OUTDIR)/Lab8.X.production.elf
SRC := src/main.c src/lcd.c
HDR := include/Configuration.h include/Lab8_Settings.h include/lcd.h
# Customize DFP_PATH if xc8-cc cannot find the device support pack.
DFP_PATH ?=
DFP_FLAG := $(if $(DFP_PATH),-mdfp="$(DFP_PATH)",)
FLAGS := -mcpu=$(MCU) $(DFP_FLAG) -Iinclude -D_XTAL_FREQ=16000000UL

.PHONY: all build clean help
all: build
build: $(OUT)
$(OUT): $(SRC) $(HDR)
	@mkdir -p "$(OUTDIR)"
	$(XC8) $(FLAGS) -o "$(ELF)" $(SRC)
clean:
	rm -rf build dist
help:
	@echo 'Build: make build [XC8=/path/to/xc8-cc] [DFP_PATH=/path/to/xc8]'
	@echo 'Target: PIC18F46K22, Fosc 16 MHz, _XTAL_FREQ=16000000UL'
