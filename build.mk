LIBUTILS_CONFIG=$(PWD)/lib/libutils.conf
LIBUTILS=$(BUILD)/lib/libutils.a

BUILD_DIRS=$(BUILD) \
		   $(BUILD)/lib \
		   $(BUILD)/bin \
		   $(OBJ_DIR) \
		   $(BUILD)/gen

TARGET=$(BUILD)/bin/mesh
MAN_PAGE=$(PWD)/doc/mesh.man
PREFIX?=/usr

SUBDIRS=$(shell cd $(PWD)/src && find * -type d)
MKSUBDIRS=$(addprefix $(OBJ_DIR)/, $(SUBDIRS))
SRCS=$(shell cd $(PWD)/src && find * -type f -name '*.c')
OBJS=$(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

FLEX_OBJ=$(OBJ_DIR)/grammar/lex.yy.o
YACC_OBJ=$(OBJ_DIR)/grammar/y.tab.o

.PHONY: build
build: $(BUILD) $(TARGET)

# Templates
define make_build_dir
$(1):
	mkdir -p $$@
endef

define mk_subdir
$(OBJ_DIR)/$(1): $(BUILD)
	mkdir -p $$@
endef

define compile_subdir
$(OBJ_DIR)/$(1)%.o: $(PWD)/src/$(1)%.c $(LIBUTILS) $(OBJ_DIR)/$(1)
	$$(CC) $$(CFLAGS) -c -o $$@ $$<
endef

# Build directory rules
$(foreach build_dir, $(BUILD_DIRS), \
	$(eval $(call make_build_dir,$(build_dir))))

$(YACC_OBJ): $(YACC_INPUT) $(BUILD)/gen
	cp -v src/grammar/ast.h $(BUILD)/gen
	cd $(BUILD)/gen && $(YACC) $(YACC_FLAGS) $<
	$(CC) $(CFLAGS_GEN) $(CFLAGS) -w -c -o $@ $(BUILD)/gen/y.tab.c

$(FLEX_OBJ): $(FLEX_INPUT) $(YACC_OBJ) $(BUILD)/gen
	cd $(BUILD)/gen && $(FLEX) $<
	$(CC) $(CFLAGS_GEN) $(CFLAGS) -w -c -o $@ $(BUILD)/gen/lex.yy.c

.PHONY: $(LIBUTILS)
$(LIBUTILS): lib/c-utils
	$(MAKE) -C $< $(TASK) \
		CONFIG_PATH=$(LIBUTILS_CONFIG) \
		BUILD=$(BUILD)

$(TARGET): $(BUILD) $(LIBUTILS) $(OBJS) $(YACC_OBJ) $(FLEX_OBJ)
	$(CC) -o $@ $(OBJS) $(YACC_OBJ) $(FLEX_OBJ) $(LDFLAGS)

# Build root
$(eval $(call compile_subdir,))

# Build subdirectories
$(foreach subdir, $(SUBDIRS), $(eval $(call mk_subdir,$(subdir))))
$(foreach subdir, $(SUBDIRS), $(eval $(call compile_subdir,$(subdir))))
