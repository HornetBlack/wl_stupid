DIRS := build test

# Wayland protocol headers and marshalling code.
PROT := $(wildcard *.xml)
PROT_HEADERS := $(PROT:%.xml=%-client-protocol.h)
PROT_SRC := $(PROT:%.xml=%-protocol.c)

HEADERS := $(wildcard *.h)
BIN := simple
SRC := $(wildcard *.c) $(PROT_SRC)
OBJ := $(SRC:%.c=build/%.o)

TEST_SRC := $(wildcard *_test.c)
TEST_BIN := $(TEST_SRC:%.c=test/%)

NON_TEST := $(shell echo $(OBJ) | sed -E 's/\S*_test.o//g')

CC := gcc -fdiagnostics-color=auto
CFLAGS := -g -Wall -Werror -O3
LDFLAGS := $(shell pkg-config --libs wayland-client) -lm

.PHONY: all
all: check_dirs $(PROT_HEADERS) $(PROT_SRC) $(OBJ) $(TEST_BIN) $(BIN)
	@echo SRC=$(SRC)
	@echo OBJ=$(OBJ)

simple: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%-client-protocol.h: %.xml
	wayland-scanner client-header < $< > $@
%-protocol.c: %.xml
	wayland-scanner code < $< > $@

build/%.o: %.c $(PROT_HEADERS) $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

test/%: build/%.o $(NON_TEST)
	$(CC) -o $@ $^
	@if $@; then echo [PASS] $@; else [FAIL] $@; fi;

.PHONY: check_dirs
check_dirs:
	mkdir -p $(DIRS)

.PHONY: clean
clean:
	rm -rf $(DIRS)
	rm $(PROT_HEADERS) $(PROT_SRC)
