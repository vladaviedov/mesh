/**
 * @file ext/store.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief Mesh context store access.
 */
#define _POSIX_C_SOURCE 200809L
#include "store.h"

#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../util/helper.h"

#define REQ_DIRECT "#:name "

// Cannot include DT_ values in a portable way
// These values are correct on Linux and BSD variants
#ifdef _DIRENT_HAVE_D_TYPE
#define DT_UNKNOWN 0
#define DT_REG 8
#endif // _DIRENT_HAVE_D_TYPE

static store_item_vector *store_list = NULL;

int store_load(const char *path) {
	if (store_list != NULL) {
		vec_delete(store_list);
	}
	store_list = vec_new(sizeof(store_item));

	DIR *store_dir = opendir(path);
	if (store_dir == NULL) {
		print_error("cannot open '%s': %s\n", path, strerror(errno));
		return -1;
	}

	int has_error = 0;

	struct dirent *item;
	while ((item = readdir(store_dir)) != NULL) {
		int skip_stat = 0;

#ifdef _DIRENT_HAVE_D_TYPE
		// Using d_type allows us to skip a stat on OS/FS that support it
		switch (item->d_type) {
		case DT_REG:
			skip_stat = 1;
			break;
		case DT_UNKNOWN:
			break;
		default:
			continue;
		}
#endif // _DIRENT_HAVE_D_TYPE

		// Make full path
		char *full_path = path_combine(path, item->d_name);

		// Portable implementation
		if (!skip_stat) {
			struct stat file_info;
			stat(full_path, &file_info);

			if (!S_ISREG(file_info.st_mode)) {
				continue;
			}
		}

		// Peek into file
		FILE *fp = fopen(full_path, "r");
		if (fp == NULL) {
			print_error("failed to load '%s'. skipping...\n", full_path);
			has_error = 1;
			continue;
		}

		char *line = NULL;
		size_t _;
		if (getline(&line, &_, fp) <= 0) {
			print_error("failed to open '%s'. skipping...\n", full_path);
			fclose(fp);
			has_error = 1;
			continue;
		}

		// Remove newline
		char *nl = strchr(line, '\n');
		if (nl != NULL) {
			*nl = '\0';
		}

		if (strncmp(line, REQ_DIRECT, strlen(REQ_DIRECT)) != 0) {
			print_error("'%s' has a bad header. skipping...\n", full_path);
			has_error = 1;
			goto next_file;
		}

		char *name = line + strlen(REQ_DIRECT);
		if (name[0] == '_') {
			print_error("'%s' has a reserved name. skipping...\n", full_path);
			has_error = 1;
			goto next_file;
		}

		store_item s = {
			.filename = full_path,
			.name = strdup(name),
		};

		vec_push(store_list, &s);

next_file:
		fclose(fp);
		free(line);
	}

	closedir(store_dir);
	return has_error ? -1 : 0;
}

const store_item *store_get(const char *name) {
	if (store_list == NULL) {
		char *path = config_path();
		store_load(path);
		free(path);
	}

	// TODO: replace with better search
	for (uint32_t i = 0; i < store_list->count; i++) {
		const store_item *item = vec_at(store_list, i);
		if (strcmp(item->name, name) == 0) {
			return item;
		}
	}

	return NULL;
}

const store_item_vector *store_get_list(void) {
	if (store_list == NULL) {
		char *path = config_path();
		store_load(path);
		free(path);
	}

	return store_list;
}
