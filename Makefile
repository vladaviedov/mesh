CC=gcc
CFLAGS=-Wall -Wextra -g
LDFLAGS=

OUT=build/shell
SUBDIRS=$(shell cd src && find * -type d -printf "%p/\n")
MKSUBDIRS=$(addprefix build/, $(SUBDIRS))
SRCS=$(shell cd src && find * -type f -name '*.c')
OBJS=$(addprefix build/, $(SRCS:.c=.o))

.PHONY:
all: build $(MKSUBDIRS) $(OUT)

$(OUT): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

build:
	mkdir -p build

# Mkdir template
define mk_subdir
build/$(1):
	mkdir -p $$@
endef

# Build template
define compile_subdir
build/$(1)%.o: src/$(1)%.c
	$(CC) $(CFLAGS) -c $$< -o $$@
endef

# Build root
$(eval $(call mk_subdir,))
$(eval $(call compile_subdir,))

# Build subdirectories
$(foreach subdir, $(SUBDIRS), $(eval $(call mk_subdir,$(subdir))))
$(foreach subdir, $(SUBDIRS), $(eval $(call compile_subdir,$(subdir))))

.PHONY: clean
clean:
	rm -rf build
