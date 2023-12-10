#include "helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define ERROR_PREFIX "mesh: "

void *ntmalloc(size_t count, size_t type_size) {
	void *ptr = malloc((count + 1) * type_size);
	memset(ptr + count * type_size, 0, type_size);
	return ptr;
}

void print_error(const char *format, ...) {
	va_list args;
	va_start(args, format);

	fprintf(stderr, ERROR_PREFIX);
	vfprintf(stderr, format, args);

	va_end(args);
}
