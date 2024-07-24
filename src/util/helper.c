#include "helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define ERROR_PREFIX "mesh: "
#define WARNING_PREFIX "warning: "

char null_char = '\0';

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

void print_warning(const char *format, ...) {
	va_list args;
	va_start(args, format);

	fprintf(stderr, WARNING_PREFIX);
	vfprintf(stderr, format, args);

	va_end(args);
}

void free_with_elements(vector *vec) {
	uint32_t count = vec->count;
	void **data = vec_collect(vec);
	for (uint32_t i = 0; i < count; i++) {
		free(data[i]);
	}

	free(data);
	vec_delete(vec);
}
