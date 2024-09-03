export PWD=$(shell pwd)
export BUILD=$(PWD)/build
export MAN_DIR=$(BUILD)/share/man/man1
export VERSION='"$(shell git describe --tags --dirty)"'

export CC=gcc
export CFLAGS=-I$(BUILD)/include -std=c99 -DMESH_VERSION=$(VERSION)
export CFLAGS_RELEASE=-O2 -w
export CFLAGS_DEBUG=-Wall -Wextra -g -DDEBUG=1
export CFLAGS_GEN=-D_POSIX_C_SOURCE=200809L
export LDFLAGS=-L$(BUILD)/lib -lutils

export FLEX=flex
export FLEX_FLAGS=
export FLEX_INPUT=$(PWD)/src/grammar/mesh.l

export YACC=yacc
export YACC_FLAGS=-d
export YACC_INPUT=$(PWD)/src/grammar/mesh.y

export OBJ_DIR=
export TASK=

BUILD_MK=$(PWD)/build.mk

# Build tasks
.PHONY: release
release: TASK=release
release: CFLAGS+=$(CFLAGS_RELEASE)
release: OBJ_DIR=$(BUILD)/obj
release:
	$(MAKE) -f $(BUILD_MK)

.PHONY: debug
debug: TASK=debug
debug: CFLAGS+=$(CFLAGS_DEBUG)
debug: OBJ_DIR=$(BUILD)/objd
debug:
	$(MAKE) -f $(BUILD_MK)

.PHONY: install
install:
	mkdir -p $(PREFIX)/bin $(PREFIX)/share/man/man1
	cp $(TARGET) $(PREFIX)/bin
	# gzip -c $(MAN_PAGE) > $(PREFIX)/share/man/man1/mesh.1.gz

.PHONY: clean
clean:
	rm -rf $(BUILD)

# Formatting
FORMAT=clang-format
FORMAT_CHECK_FLAGS=--dry-run --Werror
FORMAT_FIX_FLAGS=-i

FORMAT_FILES=$(shell find src -type f -name '*.c' -o -name '*.h')

.PHONY: checkformat
checkformat:
	$(FORMAT) $(FORMAT_CHECK_FLAGS) $(FORMAT_FILES)

.PHONY: format
format:
	$(FORMAT) $(FORMAT_FIX_FLAGS) $(FORMAT_FILES)
