/**
 * @file util/helper.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Utility helper functions.
 */
#define _POSIX_C_SOURCE 200809L
#include "helper.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../core/vars.h"

#define FATAL_PREFIX "FATAL: "
#define ERROR_PREFIX "mesh: "
#define WARNING_PREFIX "warning: "

#define CONFIG_PATH ".config/mesh/"

char null_char = '\0';
static const char *const *argv0 = NULL;

static int try_fd_reset(void);

void set_argv0(const char *const *name) {
	argv0 = name;
}

void *ntmalloc(size_t count, size_t type_size) {
	void *ptr = malloc((count + 1) * type_size);
	memset(ptr + count * type_size, 0, type_size);
	return ptr;
}

noreturn void print_fatal_hcf(const char *format, ...) {
	int reset_result = try_fd_reset();

	va_list args;
	va_start(args, format);

	fprintf(stderr, FATAL_PREFIX);
	vfprintf(stderr, format, args);

	va_end(args);

	if (reset_result < 0) {
		exit(1);
	}

	fprintf(stderr, FATAL_PREFIX);
	fprintf(stderr, "Restart mesh? [Y/n] ");

	if (getchar() != 'n') {
		execlp(*argv0, *argv0, NULL);
		fprintf(stderr, FATAL_PREFIX);
		fprintf(stderr, "Failed to restart mesh\n");
	}

	exit(1);
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

void free_elements(vector *vec) {
	uint32_t count = vec->count;
	void **data = vec_collect(vec);
	for (uint32_t i = 0; i < count; i++) {
		free(data[i]);
	}

	free(data);
}

static int try_fd_reset(void) {
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	if (open("/dev/tty", O_RDWR) < 0 || open("/dev/tty", O_RDWR) < 0
		|| open("/dev/tty", O_RDWR) < 0) {
		return -1;
	}

	return 0;
}

char *config_path(void) {
	return path_combine(vars_get("HOME"), CONFIG_PATH);
}

char *path_combine(const char *path1, const char *path2) {
	int end_slash = (path1[strlen(path1) - 1] == '/');

	uint32_t combined_size = strlen(path1) + strlen(path2) + 1;
	if (!end_slash) {
		combined_size++;
	}

	char *combined = malloc(sizeof(char) * combined_size);
	if (!end_slash) {
		snprintf(combined, combined_size, "%s/%s", path1, path2);
	} else {
		snprintf(combined, combined_size, "%s%s", path1, path2);
	}

	return combined;
}
