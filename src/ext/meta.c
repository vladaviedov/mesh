#include "meta.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../util/helper.h"
#include "../util/vector.h"
#include "context.h"

typedef struct {
	const char *name;
	int (*const func)(uint32_t argc, char **argv, char **command);
} meta;

// Meta comamnds
int meta_asroot(uint32_t argc, char **argv, char **command);

static const meta registry[] = {
	{ .name = ":asroot", .func = &meta_asroot }
};
static const size_t registry_length = sizeof(registry) / sizeof(meta);

// Helper functions
const meta *find_meta(const char *name);

int run_meta(str_vec *args, char **command) {
	char *name = fix_ptr(vec_at(args, 0));
	const meta *result = find_meta(name);
	if (result == NULL) {
		return -1;
	}

	return result->func(args->count, args->raw, command);
}

/** Meta commands */

int meta_asroot(uint32_t argc, char **argv, char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	uint32_t index = 0;
	if (argc == 2) {
		char *end;
		index = strtol(argv[1], &end, 10);

		if (*end != '\0') {
			print_error("argument must be a row\n");
			return -1;
		}
	}

	const char *result = context_get_row_rel(index);
	if (result == NULL) {
		print_error("no such command in context\n");
		return -1;
	}

	char buffer[strlen(result) + 5];
	sprintf(buffer, "doas %s", result);

	*command = strdup(buffer);
	return 0;
}

/** Internal commands */

/**
 * @brief Find builtin in the registry.
 *
 * @param[in] name - Search query.
 * @return Builtin entry; NULL on error.
 */
const meta *find_meta(const char *name) {
	for (size_t i = 0; i < registry_length; i++) {
		if (strcmp(registry[i].name, name) == 0) {
			return registry + i;
		}
	}

	return NULL;
}
