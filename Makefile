CC=gcc
CFLAGS=-Wall -Wextra -g
LDFLAGS=

EXEC=build/shell
SRCS=$(shell cd src && find -L * -type f -name '*.c')
OBJS=$(addprefix build/, $(SRCS:.c=.o))

.PHONY: all
all: build $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

build:
	mkdir -p build

.PHONY: clean
clean:
	rm build/*
	rmdir build
