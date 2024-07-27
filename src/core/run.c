/**
 * @file core/run.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Figure out how to run a command.
 */
#define _POSIX_C_SOURCE 200809L
#include "run.h"

#include <stdlib.h>
#include <string.h>

#include <c-utils/vector.h>

#include "../ext/context.h"
#include "../ext/meta.h"
#include "../grammar/ast.h"
#include "../grammar/parse.h"
#include "../util/helper.h"
#include "builtins.h"
#include "eval.h"
#include "exec.h"

int run_dispatch(string_vector *args, run_flags *flags) {
	// Meta commands
	char *const *argv0 = vec_at(args, 0);
	if (**argv0 == ':') {
		char *meta_out;
		int meta_result = run_meta(args, &meta_out);

		if (meta_result < 0) {
			return -1;
		}
		if (meta_result == 0) {
			return 0;
		}

		ast_node *parsed = parse_from_string(meta_out);
		meta_result = eval_ast(parsed);
		ast_recurse_free(parsed);

		context_hist_add(strdup(meta_out));

		free(meta_out);
		return meta_result;
	}

	// Builtins
	int code = run_builtin(args);
	if (code >= 0) {
		return code;
	}

	// Add null-terminator to args
	char *argv[args->count + 1];
	memcpy(argv, args->data, args->count * sizeof(char *));
	argv[args->count] = NULL;

	// Exec program
	return exec_normal(argv);
}

run_flags copy_flags(const run_flags *flags) {
	run_flags new_flags = {
		.redirs = vec_init_clone(&flags->redirs),
		.assigns = vec_init_clone(&flags->assigns),
	};

	return new_flags;
}
