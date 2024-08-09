/**
 * @file ext/meta.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Mesh meta-commands.
 */
#define _POSIX_C_SOURCE 200809L
#include "meta.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c-utils/nanorl.h>
#include <c-utils/vector.h>

#include "../core/exec.h"
#include "../core/vars.h"
#include "../util/helper.h"
#include "context.h"
#include "store.h"

#define SPACE_STR " "
#define IMPORT_CTX_NAME "_import_ctx"
#define CTX_EXT ".ctx"

#define DIRECT_START "#:"
#define DIRECT_NAME "name "

// Meta comamnds
// Note: typedef in header
struct meta {
	const char *name;
	int (*const func)(uint32_t argc, char **argv, char **command);
	int hidden;
};

// Shown meta commands
static int meta_add(uint32_t argc, char **argv, unused char **command);
static int meta_replace(uint32_t argc, char **argv, unused char **command);
static int meta_ctx(uint32_t argc, char **argv, unused char **command);
static int meta_store(uint32_t argc, char **argv, unused char **command);
static int meta_asroot(uint32_t argc, char **argv, char **command);
static noreturn int meta_hcf(
	unused uint32_t argc, unused char **argv, unused char **command);
// "Hidden" meta commands
static int meta_ctx_show(uint32_t argc, char **argv, unused char **command);
static int meta_ctx_set(uint32_t argc, char **argv, unused char **command);
static int meta_ctx_ls(
	uint32_t argc, unused char **argv, unused char **command);
static int meta_ctx_make(uint32_t argc, char **argv, unused char **command);
static int meta_ctx_new(uint32_t argc, char **argv, unused char **command);
static int meta_ctx_del(uint32_t argc, char **argv, unused char **command);
static int meta_ctx_import(uint32_t argc, char **argv, unused char **command);
static int meta_ctx_export(uint32_t argc, char **argv, unused char **command);
static int meta_store_load(uint32_t argc, char **argv, unused char **command);
static int meta_store_save(uint32_t argc, char **argv, unused char **command);
static int meta_store_ls(uint32_t argc, char **argv, unused char **command);
static int meta_store_edit(uint32_t argc, char **argv, char **command);
static int meta_store_reload(uint32_t argc, char **argv, unused char **command);

static const meta registry[] = {
	{ .name = ":a", .func = &meta_add, .hidden = 0 },
	{ .name = ":add", .func = &meta_add, .hidden = 0 },
	{ .name = ":r", .func = &meta_replace, .hidden = 0 },
	{ .name = ":replace", .func = &meta_replace, .hidden = 0 },
	{ .name = ":c", .func = &meta_ctx, .hidden = 0 },
	{ .name = ":ctx", .func = &meta_ctx, .hidden = 0 },
	{ .name = ":s", .func = &meta_store, .hidden = 0 },
	{ .name = ":store", .func = &meta_store, .hidden = 0 },
	{ .name = ":asroot", .func = &meta_asroot, .hidden = 0 },
	{ .name = ":hcf", .func = &meta_hcf, .hidden = 0 },
	{ .name = ":_ctx_show", .func = &meta_ctx_show, .hidden = 1 },
	{ .name = ":_ctx_set", .func = &meta_ctx_set, .hidden = 1 },
	{ .name = ":_ctx_ls", .func = &meta_ctx_ls, .hidden = 1 },
	{ .name = ":_ctx_make", .func = &meta_ctx_make, .hidden = 1 },
	{ .name = ":_ctx_new", .func = &meta_ctx_new, .hidden = 1 },
	{ .name = ":_ctx_del", .func = &meta_ctx_del, .hidden = 1 },
	{ .name = ":_ctx_import", .func = &meta_ctx_import, .hidden = 1 },
	{ .name = ":_ctx_export", .func = &meta_ctx_export, .hidden = 1 },
	{ .name = ":_store_load", .func = &meta_store_load, .hidden = 1 },
	{ .name = ":_store_save", .func = &meta_store_save, .hidden = 1 },
	{ .name = ":_store_ls", .func = &meta_store_ls, .hidden = 1 },
	{ .name = ":_store_edit", .func = &meta_store_edit, .hidden = 1 },
	{ .name = ":_store_reload", .func = &meta_store_reload, .hidden = 1 },
};
static const size_t registry_length = sizeof(registry) / sizeof(meta);

// Helper functions
static const char *get_root_program(void);
static char *unsplit(uint32_t count, char **words);
static int load_ctx_from_file(FILE *infile, const char *filename);

const meta *search_meta(const char *name) {
	for (size_t i = 0; i < registry_length; i++) {
		if (strcmp(registry[i].name, name) == 0) {
			return registry + i;
		}
	}

	return NULL;
}

int run_meta(const meta *cmd, string_vector *args, char **command) {
	if (cmd->hidden) {
		print_warning("this command is not intended to be called directly "
					  "from the shell\n");
	}

	return cmd->func(args->count, args->data, command);
}

/** Meta commands */

static int meta_add(uint32_t argc, char **argv, unused char **command) {
	if (argc == 1) {
		nrl_error err;
		char *input = nanorl("", &err);

		switch (err) {
		case NRL_ERR_SYS:
		case NRL_ERR_BAD_FD:
			print_error("failed to get input\n");
			return -1;
		case NRL_ERR_EMPTY:
			print_warning("no input given\n");
			return -1;
		case NRL_ERR_OK:
			return context_add(input, NULL);
		}
	}

	char *combined = unsplit(argc - 1, argv + 1);
	return context_add(combined, NULL);
}

static int meta_replace(uint32_t argc, char **argv, unused char **command) {
	if (argc == 1) {
		print_error("not enough arguments\n");
		return -1;
	}

	char *end;
	int32_t item = strtol(argv[1], &end, 10);
	if (*end != '\0') {
		print_error("invalid command index\n");
		return -1;
	}

	// Relative indexing
	if (item < 0) {
		item = context_get(NULL)->commands.count + item;
	}

	if (argc == 2) {
		nrl_error err;
		char *input = nanorl("", &err);

		switch (err) {
		case NRL_ERR_SYS:
		case NRL_ERR_BAD_FD:
			print_error("failed to get input\n");
			return -1;
		case NRL_ERR_EMPTY:
			print_warning("no input given\n");
			return -1;
		case NRL_ERR_OK:
			return context_replace(input, item, NULL);
		}
	}

	char *combined = unsplit(argc - 2, argv + 2);
	return context_replace(combined, item, NULL);
}

static int meta_ctx(uint32_t argc, char **argv, unused char **command) {
	// No arguments defaults to show current context
	if (argc == 1) {
		return meta_ctx_show(1, NULL, NULL);
	}

	char sub_name[1024];
	snprintf(sub_name, 1024, ":_ctx_%s", argv[1]);
	const meta *sub = search_meta(sub_name);
	if (sub == NULL) {
		print_error("ctx subcommand '%s' does not exist\n", argv[1]);
		return -1;
	}

	return sub->func(argc - 1, argv + 1, command);
}

static int meta_store(uint32_t argc, char **argv, unused char **command) {
	// No arguments defaults to show current context
	if (argc == 1) {
		return meta_store_ls(1, NULL, NULL);
	}

	char sub_name[1024];
	snprintf(sub_name, 1024, ":_store_%s", argv[1]);
	const meta *sub = search_meta(sub_name);
	if (sub == NULL) {
		print_error("store subcommand '%s' does not exist\n", argv[1]);
		return -1;
	}

	return sub->func(argc - 1, argv + 1, command);
}

static int meta_asroot(uint32_t argc, char **argv, char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	int32_t index = -1;
	if (argc == 2) {
		char *end;
		index = strtol(argv[1], &end, 10);

		if (*end != '\0') {
			print_error("argument must be a row\n");
			return -1;
		}
	}

	const char *result = context_get_row(index);
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

	char buffer[strlen(result) + strlen(root_program) + 2];
	sprintf(buffer, "%s %s", root_program, result);

	*command = strdup(buffer);
	return 1;
}

static noreturn int meta_hcf(
	unused uint32_t argc, unused char **argv, unused char **command) {
	print_fatal_hcf("user-invoked crash\n");
}

/** Hidden meta commands */

static int meta_ctx_show(uint32_t argc, char **argv, unused char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	// Get context
	const context *ctx = (argc == 2) ? context_get(argv[1]) : context_get(NULL);

	if (ctx == NULL) {
		if (argc == 2) {
			print_error("context '%s' not found\n", argv[1]);
		}

		return -1;
	}

	// Print context information
	printf("Context name: %s\n\n", ctx->name);
	for (uint32_t i = 0; i < ctx->commands.count; i++) {
		printf("%u: %s\n", i, *(char *const *)vec_at(&ctx->commands, i));
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

static int meta_ctx_ls(
	uint32_t argc, unused char **argv, unused char **command) {
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

	// Reserved
	if (argv[1][0] == '_') {
		print_error("context names may not begin with an underscore\n");
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

static uint32_t stdin_import_counter = 0;

static int meta_ctx_import(uint32_t argc, char **argv, unused char **command) {
	// Import from stdin
	if (argc == 1) {
		char default_name[32];
		snprintf(default_name, 32, "stdin_%u", stdin_import_counter);
		if (load_ctx_from_file(stdin, default_name) < 0) {
			return -1;
		}

		stdin_import_counter++;
		return 0;
	}

	int some_failed = 0;
	for (uint32_t i = 1; i < argc; i++) {
		FILE *source = fopen(argv[i], "r");
		if (load_ctx_from_file(source, argv[i]) < 0) {
			some_failed = 1;
		}
	}

	return some_failed ? -1 : 0;
}

static int meta_ctx_export(uint32_t argc, char **argv, unused char **command) {
	if (argc > 3) {
		print_error("too many arguments\n");
		return -1;
	}

	char *ctx_name = (argc == 1) ? NULL : argv[1];

	const context *ctx = context_get(ctx_name);
	if (ctx == NULL) {
		print_error("context '%s' is not loaded\n");
		return -1;
	}

	FILE *out_file;
	int need_close = 0;
	if (argc <= 2) {
		out_file = stdout;
	} else {
		out_file = fopen(argv[2], "w");
		if (out_file == NULL) {
			print_error("failed to open '%s': %s\n", argv[2], strerror(errno));
			return -1;
		}

		need_close = 1;
	}

	// Print header
	fprintf(out_file, "%s%s%s\n", DIRECT_START, DIRECT_NAME, ctx->name);

	// Write commands
	for (uint32_t i = 0; i < ctx->commands.count; i++) {
		const char *const *command = vec_at(&ctx->commands, i);
		fprintf(out_file, "%s\n", *command);
	}

	if (need_close) {
		fclose(out_file);
	}

	return 0;
}

static int meta_store_load(uint32_t argc, char **argv, unused char **command) {
	if (argc == 1) {
		print_error("too few arguments\n");
		return -1;
	}

	char *import_argv[argc];
	import_argv[0] = argv[0];

	for (uint32_t i = 1; i < argc; i++) {
		const store_item *item = store_get(argv[i]);
		if (item == NULL) {
			print_error("'%s' not found in store\n", argv[i]);
			return -1;
		}

		import_argv[i] = item->filename;
	}

	return meta_ctx_import(argc, import_argv, command);
}

static int meta_store_save(uint32_t argc, char **argv, unused char **command) {
	if (argc < 2) {
		print_error("too few arguments\n");
		return -1;
	}

	char *export_argv[3];
	export_argv[0] = argv[0];

	for (uint32_t i = 1; i < argc; i++) {
		const store_item *item = store_get(argv[i]);

		char *filepath;
		if (item == NULL) {
			char *config = config_path();
			char *no_ext = path_combine(config, argv[i]);

			filepath = malloc(sizeof(char) * (strlen(no_ext) + 5));
			sprintf(filepath, "%s%s", no_ext, CTX_EXT);

			free(no_ext);
			free(config);
		} else {
			filepath = item->filename;
		}

		export_argv[1] = argv[i];
		export_argv[2] = filepath;

		int res = meta_ctx_export(3, export_argv, command);

		if (res < 0) {
			fprintf(stderr, "failed to save '%s'\n", argv[i]);

			if (item == NULL) {
				free(filepath);
			}

			return res;
		}

		printf("saved '%s' as '%s'\n", argv[i], filepath);

		if (item == NULL) {
			free(filepath);
		}
	}

	return meta_store_reload(1, NULL, NULL);
}

static int meta_store_ls(
	uint32_t argc, unused char **argv, unused char **command) {
	if (argc != 1) {
		print_error("too many arguments\n");
		return -1;
	}

	const store_item_vector *store = store_get_list();
	for (uint32_t i = 0; i < store->count; i++) {
		const store_item *item = vec_at(store, i);
		printf("%s (%s)\n", item->name, item->filename);
	}

	return 0;
}

static int meta_store_edit(uint32_t argc, char **argv, char **command) {
	if (argc != 2) {
		print_error("invalid argument count\n");
		return -1;
	}

	const store_item *item = store_get(argv[1]);
	if (item == NULL) {
		print_error("'%s' not found in store\n", argv[1]);
		return -1;
	}

	const char *editor = vars_get("EDITOR");
	if (editor == NULL) {
		print_error("EDITOR environment variable is not set\n");
		return -1;
	}

	char buffer[strlen(editor) + strlen(item->filename) + 2];
	sprintf(buffer, "%s %s", editor, item->filename);
	*command = strdup(buffer);
	return 1;
}

static int meta_store_reload(
	uint32_t argc, unused char **argv, unused char **command) {
	if (argc != 1) {
		print_error("too mary arguments\n");
		return -1;
	}

	char *path = config_path();
	int res = store_load(path);
	free(path);
	return res;
}

/** Internal commands */

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

/**
 * @brief Recombine a split argv back into 1 string.
 *
 * @param[in] count - Word count.
 * @param[in] words - Word vector.
 * @return Recombined string.
 * @note Allocated return value.
 */
static char *unsplit(uint32_t count, char **words) {
	// Measure length of the combined string
	uint32_t new_size = 0;
	for (uint32_t i = 0; i < count; i++) {
		new_size += strlen(words[i]) + 1;
	}

	// Combine into one string
	char *combined = malloc(sizeof(char) * new_size);
	combined[0] = '\0';

	// First element
	strcat(combined, words[0]);
	for (uint32_t i = 1; i < count; i++) {
		strcat(combined, SPACE_STR);
		strcat(combined, words[i]);
	}

	return combined;
}

static int load_ctx_from_file(FILE *infile, const char *filename) {
	char *line = NULL;
	size_t _ = 0;

	context *ctx;
	if (context_new(IMPORT_CTX_NAME, &ctx) < 0) {
		print_error("failed to create a new context\n");
		return -1;
	}

	int error = 0;
	while (!error && getline(&line, &_, infile) > 0) {
		// Remove newline and spaces at the end
		char *nl = strchr(line, '\n');
		if (nl != NULL) {
			*nl = '\0';
		}
		while (--nl > line) {
			char ch = *nl;

			if (ch == ' ' || ch == '\t') {
				*nl = '\0';
			} else {
				break;
			}
		}

		// Trim leading spaces
		char *trimmed = line;
		while (*trimmed != '\0') {
			char ch = *trimmed;

			if (ch == ' ' || ch == '\t') {
				trimmed++;
			} else {
				break;
			}
		}

		// Parse directives
		if (strncmp(trimmed, DIRECT_START, strlen(DIRECT_START)) == 0) {
			if (strncmp(trimmed + strlen(DIRECT_START), DIRECT_NAME,
					strlen(DIRECT_NAME))
				== 0) {
				uint32_t name_start
					= strlen(DIRECT_START) + strlen(DIRECT_NAME);
				if (strlen(trimmed) < name_start) {
					print_error("%s: no argument to name", filename);
					error = 1;
					goto line_cleanup;
				}

				// Reserved
				if (trimmed[name_start] == '_') {
					print_error("%s: invalid context name\n", filename);
					error = 1;
					goto line_cleanup;
				}

				free(ctx->name);
				ctx->name = strdup(trimmed + name_start);
			} else {
				print_error(
					"%s: invalid directive '%s'\n", filename, trimmed + 2);
				error = 1;
				goto line_cleanup;
			}
		}

		// Ignore comment and empty lines
		if (strlen(trimmed) == 0 || trimmed[0] == '#') {
			goto line_cleanup;
		}

		context_add(strdup(trimmed), ctx);

line_cleanup:
		free(line);
		line = NULL;
	}

	if (line != NULL) {
		free(line);
	}

	if (error) {
		print_error("failed to import from %s\n", filename);
		context_delete(IMPORT_CTX_NAME);
		return -1;
	}

	// If name is not modified, use filename
	if (ctx->name[0] == '_') {
		free(ctx->name);
		ctx->name = strdup(filename);
	}

	printf("imported '%s' as '%s'\n", filename, ctx->name);
	return 0;
}
