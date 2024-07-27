/**
 * @file ext/meta.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Mesh meta-commands.
 */
#include "meta.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c-utils/vector.h>

#include "../util/helper.h"
#include "../core/vars.h"
#include "../core/exec.h"
#include "context.h"

static int abs_index = 0;

// Meta comamnds
typedef struct {
	const char *name;
	int (*const func)(uint32_t argc, char **argv, char **command);
	int hidden;
} meta;

// Shown meta commands
static int meta_ctx(uint32_t argc, char **argv, unused char **command);
static int meta_abs(uint32_t argc, char **argv, unused char **command);
static int meta_asroot(uint32_t argc, char **argv, char **command);
// "Hidden" meta commands
static int meta_ctx_show(uint32_t argc, char **argv, unused char **command);
static int meta_ctx_set(uint32_t argc, char **argv, unused char **command);
static int meta_ctx_ls(uint32_t argc, unused char **argv, unused char **command);
static int meta_ctx_make(uint32_t argc, char **argv, unused char **command);
static int meta_ctx_new(uint32_t argc, char **argv, unused char **command);
static int meta_ctx_del(uint32_t argc, char **argv, unused char **command);

static const meta registry[] = {
	{ .name = ":ctx", .func = &meta_ctx, .hidden = 0 },
	{ .name = ":abs", .func = &meta_abs, .hidden = 0 },
	{ .name = ":asroot", .func = &meta_asroot, .hidden = 0 },
	{ .name = ":_ctx_show", .func = &meta_ctx_show, .hidden = 1 },
	{ .name = ":_ctx_set", .func = &meta_ctx_set, .hidden = 1 },
	{ .name = ":_ctx_ls", .func = &meta_ctx_ls, .hidden = 1 },
	{ .name = ":_ctx_make", .func = &meta_ctx_make, .hidden = 1 },
	{ .name = ":_ctx_new", .func = &meta_ctx_new, .hidden = 1 },
	{ .name = ":_ctx_del", .func = &meta_ctx_del, .hidden = 1 }
};
static const size_t registry_length = sizeof(registry) / sizeof(meta);

// Helper functions
static const meta *find_meta(const char *name);
static const char *get_root_program(void);

int run_meta(string_vector *args, char **command) {
	// Check for standard meta
	char *const *name = vec_at(args, 0);
	const meta *result = find_meta(*name);
	if (result != NULL) {
		if (result->hidden) {
			print_warning("this command is not intended to be called directly "
				"from the shell\n");
		}

		return result->func(args->count, args->data, command);
	}

	// Check for numeric meta
	char *end;
	uint32_t index = strtoul(*name + 1, &end, 10);
	if (*end != '\0') {
		print_error("%s: meta command not found\n", *name + 1);
		return -1;
	}

	const char *num_result = abs_index
		? context_get_row_abs(index)
		: context_get_row_rel(index);
	if (num_result == NULL) {
		print_error("no such command in context\n");
		return -1;
	}

	*command = strdup(num_result);
	return 1;
}

/** Meta commands */

static int meta_ctx(uint32_t argc, char **argv, unused char **command) {
	// No arguments defaults to show current context
	if (argc == 1) {
		return meta_ctx_show(1, NULL, NULL);
	}

	char sub_name[1024];
	snprintf(sub_name, 1024, ":_ctx_%s", argv[1]);
	const meta *sub = find_meta(sub_name);
	if (sub == NULL) {
		print_error("ctx subcommand '%s' does not exist\n", argv[1]);
		return -1;
	}

	return sub->func(argc - 1, argv + 1, command);
}

static int meta_abs(uint32_t argc, char **argv, unused char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	if (argc == 2) {
		switch (argv[1][0]) {
			case '0':
				abs_index = 0;
				break;
			case '1':
				abs_index = 1;
				break;
			default:
				print_error("invalid argument\n");
				return -1;
		}
	} else {
		abs_index = !abs_index;
	}

	if (abs_index) {
		printf("using absolute indexing\n");
	} else {
		printf("using relative indexing\n");
	}

	return 0;
}

static int meta_asroot(uint32_t argc, char **argv, char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	uint32_t index = 0;
	if (argc == 2) {
		char *end;
		index = strtoul(argv[1], &end, 10);

		if (*end != '\0') {
			print_error("argument must be a row\n");
			return -1;
		}
	}

	const char *result = abs_index
		? context_get_row_abs(index)
		: context_get_row_rel(index);
	if (result == NULL) {
		print_error("no such command in context\n");
		return -1;
	}

	// Find 'doas' or 'sudo'
	const char *root_program = get_root_program();
	if (root_program == NULL) {
		print_error("cannot find doas or sudo on your system\n");
		return 1;
	}

	char buffer[strlen(result) + strlen(root_program) + 1];
	sprintf(buffer, "%s %s", root_program, result);

	*command = strdup(buffer);
	return 1;
}

/** Hidden meta commands */

static int meta_ctx_show(uint32_t argc, char **argv, unused char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	// Get context
	const context *ctx = (argc == 2)
		? context_get(argv[1])
		: context_get(NULL);

	if (ctx == NULL) {
		if (argc == 2) {
			print_error("context '%s' not found\n", argv[1]);
		}

		return -1;
	}

	// Print context information
	printf("Context name: %s\n\n", ctx->name);
	for (uint32_t i = 0; i < ctx->commands.count; i++) {
		uint32_t index = abs_index ? i : ctx->commands.count - i - 1;
		printf("%u: %s\n", index, *(char *const *)vec_at(&ctx->commands, i));
	}

	return 0;
}

static int meta_ctx_set(uint32_t argc, char **argv, unused char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	if (argc < 2) {
		print_error("context not specified\n");
		return -1;
	}

	if (context_select(argv[1]) < 0) {
		print_error("context '%s' not found\n");
		return -1;
	}

	printf("switched to '%s'\n", argv[1]);
	return 0;
}

static int meta_ctx_ls(uint32_t argc, unused char **argv, unused char **command) {
	if (argc > 1) {
		print_error("too many arguments\n");
		return -1;
	}

	const context *current_ctx = context_get(NULL);
	const context_vector *all_ctxs = context_get_all();
	
	if (all_ctxs == NULL) {
		print_error("context system not initialized\n");
		return -1;
	}

	for (uint32_t i = 0; i < all_ctxs->count; i++) {
		const context *ctx = vec_at(all_ctxs, i);
		printf("%s", ctx->name);

		if (ctx == current_ctx) {
			printf(" (selected)\n");
		} else {
			printf("\n");
		}
	}

	return 0;
}

static int meta_ctx_make(uint32_t argc, char **argv, unused char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	if (argc < 2) {
		print_error("new context requires a name\n");
		return -1;
	}

	if (context_new(argv[1], NULL) < 0) {
		print_error("context already exists\n");
		return -1;
	}

	printf("created new context '%s'\n", argv[1]);
	return 0;
}

static int meta_ctx_new(uint32_t argc, char **argv, unused char **command) {
	if (meta_ctx_make(argc, argv, command) < 0) {
		return -1;
	}
	if (meta_ctx_set(argc, argv, command) < 0) {
		return -1;
	}

	return 0;
}

static int meta_ctx_del(uint32_t argc, char **argv, unused char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	if (argc < 2) {
		print_error("new context requires a name\n");
		return -1;
	}

	if (context_delete(argv[1]) < 0) {
		print_error("failed to delete '%s'\n", argv[1]);
		return -1;
	}

	printf("context '%s' deleted\n", argv[1]);
	return 0;
}

/** Internal commands */

/**
 * @brief Find builtin in the registry.
 *
 * @param[in] name - Search query.
 * @return Builtin entry; NULL on error.
 */
static const meta *find_meta(const char *name) {
	for (size_t i = 0; i < registry_length; i++) {
		if (strcmp(registry[i].name, name) == 0) {
			return registry + i;
		}
	}

	return NULL;
}

static const char *root_prog;
/**
 * @brief Get an installed program for becoming root.
 *
 * @return Name of program; NULL on error.
 */
static const char *get_root_program(void) {
	if (root_prog != NULL) {
		return root_prog;
	}

	// Check environment
	if ((root_prog = vars_get("ASROOTCMD")) != NULL) {
		return root_prog;
	}

	// Check for 'doas'
	char *doas_check[] = { "which", "doas", NULL };
	if (exec_silent(doas_check) == 0) {
		root_prog = "doas";
		return root_prog;
	}
	
	// Check for 'sudo'
	char *sudo_check[] = { "which", "sudo", NULL };
	if (exec_silent(sudo_check) == 0) {
		root_prog = "sudo";
		return root_prog;
	}

	return NULL;
}
