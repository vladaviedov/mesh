/**
 * @file util/error.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief Error handling defines and functions.
 */
#define _POSIX_C_SOURCE 200809L
#include "error.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "helper.h"

#define MESH_PREFIX "[mesh] "

#define FATAL_PREFIX "fatal: "
#define ERROR_PREFIX "error: "
#define WARNING_PREFIX "warn: "

static const char *const *argv0 = NULL;

static int try_fd_reset(void);

void set_argv0(const char *const *name) {
	argv0 = name;
}

noreturn void print_fatal_hcf(const char *format, ...) {
	int reset_result = try_fd_reset();

	va_list args;
	va_start(args, format);

	fprintf(stderr, MESH_PREFIX);
	fprintf(stderr, FATAL_PREFIX);
	vfprintf(stderr, format, args);

	va_end(args);

	if (reset_result < 0) {
		exit(1);
	}

	fprintf(stderr, MESH_PREFIX);
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

	fprintf(stderr, MESH_PREFIX);
	fprintf(stderr, ERROR_PREFIX);

	vfprintf(stderr, format, args);

	va_end(args);
}

void print_warning(const char *format, ...) {
	va_list args;
	va_start(args, format);

	fprintf(stderr, MESH_PREFIX);
	fprintf(stderr, WARNING_PREFIX);
	vfprintf(stderr, format, args);

	va_end(args);
}

/**
 * @brief Reset standard file descriptors to the user's terminal.
 *
 * @return 0 on success; -1 on failure.
 */
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
