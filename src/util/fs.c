/**
 * @file util/fs.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief Filesystem utility functions.
 */
#define _POSIX_C_SOURCE 200809L
#include "fs.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../core/vars.h"

#define CONF_ROOT ".config/mesh/"
#define CONF_CTX (CONF_ROOT "ctx/")

static char *path_conf_ctx = NULL;

static int mkdir_p(char *path);

void fs_path_cat(char *path1, const char *path2) {
	uint32_t path1_len = strlen(path1);
	uint32_t path2_len = strlen(path2);

	// Only add slash if there isn't one already - try to avoid double slashes
	char path1_ending = path1[path1_len - 1];
	if (path1_ending != '/') {
		path1[path1_len] = '/';
		path1_len++;
	}

	// Theoretically, doing this without 'strcat' saves some 'strlen' calls
	memcpy(path1 + path1_len, path2, path2_len);

	// Add null-terminator
	path1[path1_len + path2_len] = '\0';
}

char *fs_path_malloc(void) {
	return malloc(sizeof(char) * PATH_MAX);
}

int fs_mkdir_p(const char *path) {
	char *path_copy = strdup(path);
	int res = mkdir_p(path_copy);

	free(path_copy);
	return res;
}

const char *fs_conf_ctx(void) {
	if (path_conf_ctx == NULL) {
		path_conf_ctx = malloc(sizeof(char) * PATH_MAX);
		strcpy(path_conf_ctx, vars_get("HOME"));

		fs_path_cat(path_conf_ctx, CONF_CTX);
	}

	return path_conf_ctx;
}

/**
 * @brief Recursive helper for 'fs_mkdir_p'.
 *
 * @param[in] path - Copied path.
 * @return 0 on success; -1 on error.
 */
static int mkdir_p(char *path) {
	char *last_slash = strrchr(path, '/');

	// If there is a slash and it's not the root slash, need to recurse higher
	if (last_slash != NULL && last_slash != path) {
		*last_slash = '\0';

		if (mkdir_p(path) < 0) {
			return -1;
		}

		// Restore path
		*last_slash = '/';
	}

	// If already exists, it's fine
	if (mkdir(path, 0755) == 0 || errno == EEXIST) {
		return 0;
	}

	return -1;
}
