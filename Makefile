PWD=$(shell pwd)
BUILD=$(PWD)/build
MAN_DIR=$(BUILD)/share/man/man1

CC=gcc
CFLAGS=-I$(BUILD)/include -std=gnu99
CFLAGS_RELEASE=-O2
CFLAGS_DEBUG=-Wall -Wextra -g -DDEBUG=1
LDFLAGS=-L$(BUILD)/lib -lutils

# Is POSIX lex possible here?
FLEX=flex
FLEX_INPUT=$(PWD)/src/grammar/mesh.l
FLEX_OBJ=$(BUILD)/obj/grammar/lex.yy.o
YACC=yacc
YACC_FLAGS=-d
YACC_INPUT=$(PWD)/src/grammar/mesh.y
YACC_OBJ=$(BUILD)/obj/grammar/y.tab.o

LIBUTILS_CONFIG=$(PWD)/lib/libutils.conf
LIBUTILS=$(BUILD)/lib/libutils.a

TARGET=$(BUILD)/bin/mesh
MAN_PAGE=$(PWD)/doc/mesh.man
PREFIX?=/usr

SUBDIRS=$(shell cd $(PWD)/src && find * -type d)
MKSUBDIRS=$(addprefix $(BUILD)/obj/, $(SUBDIRS))
SRCS=$(shell cd $(PWD)/src && find * -type f -name '*.c')
OBJS=$(addprefix $(BUILD)/obj/, $(SRCS:.c=.o))

# Templates
define mk_subdir
$(BUILD)/obj/$(1): $(BUILD)
	mkdir -p $$@
endef

define compile_subdir
$(BUILD)/obj/$(1)%.o: $(PWD)/src/$(1)%.c $(LIBUTILS) $(BUILD)/obj/$(1)
	$$(CC) $$(CFLAGS) -c -o $$@ $$<
endef

# Build tasks
.PHONY: release
release: TASK=release
release: CFLAGS+=$(CFLAGS_RELEASE)
release: build

.PHONY: debug
debug: TASK=debug
debug: CFLAGS+=$(CFLAGS_DEBUG)
debug: build

.PHONY: install
install:
	mkdir -p $(PREFIX)/bin $(PREFIX)/share/man/man1
	cp $(TARGET) $(PREFIX)/bin
	# gzip -c $(MAN_PAGE) > $(PREFIX)/share/man/man1/mesh.1.gz

.PHONY: build
build: $(BUILD) $(TARGET)

.PHONY: clean
clean:
	rm -rf $(BUILD)

$(BUILD):
	mkdir -p $(BUILD)
	mkdir -p $(BUILD)/bin
	mkdir -p $(BUILD)/gen

$(YACC_OBJ): $(YACC_INPUT) $(BUILD) $(BUILD)/obj/grammar
	cp -v src/grammar/ast.h $(BUILD)/gen
	cd $(BUILD)/gen && $(YACC) $(YACC_FLAGS) $<
	$(CC) $(CFLAGS) -w -c -o $@ $(BUILD)/gen/y.tab.c

$(FLEX_OBJ): $(FLEX_INPUT) $(YACC_OBJ) $(BUILD) $(BUILD)/obj/grammar
	cd $(BUILD)/gen && $(FLEX) $<
	$(CC) $(CFLAGS) -w -c -o $@ $(BUILD)/gen/lex.yy.c

$(LIBUTILS): lib/c-utils
	$(MAKE) -C $< $(TASK) \
		CONFIG_PATH=$(LIBUTILS_CONFIG) \
		BUILD=$(BUILD)

$(TARGET): $(BUILD) $(LIBUTILS) $(OBJS) $(YACC_OBJ) $(FLEX_OBJ)
	$(CC) -o $@ $(OBJS) $(YACC_OBJ) $(FLEX_OBJ) $(LDFLAGS)

# Build root
$(eval $(call mk_subdir,))
$(eval $(call compile_subdir,))

# Build subdir
# ectories
$(foreach subdir, $(SUBDIRS), $(eval $(call mk_subdir,$(subdir))))
$(foreach subdir, $(SUBDIRS), $(eval $(call compile_subdir,$(subdir))))

# Formatting
FORMAT=clang-format
FORMAT_CHECK_FLAGS=--dry-run --Werror
FORMAT_FIX_FLAGS=-i

FORMAT_FILES=$(shell find src -type f)

.PHONY: checkformat
checkformat:
	$(FORMAT) $(FORMAT_CHECK_FLAGS) $(FORMAT_FILES)

.PHONY: format
format:
	$(FORMAT) $(FORMAT_FIX_FLAGS) $(FORMAT_FILES)
