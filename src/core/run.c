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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <c-utils/vector.h>

#include "../ext/context.h"
#include "../ext/meta.h"
#include "../grammar/ast.h"
#include "../grammar/parse.h"
#include "../util/error.h"
#include "../util/helper.h"
#include "builtins.h"
#include "eval.h"
#include "exec.h"
#include "flags.h"

int run_dispatch(string_vector *args, run_flags *flags) {
	// Meta commands
	char *const *argv0 = vec_at(args, 0);
	if (**argv0 == ':') {
		char *meta_out;
		const meta *meta_cmd = search_meta(*argv0);
		if (meta_cmd == NULL) {
			print_error("%s: meta command not found\n", *argv0 + 1);
			return -1;
		}

		if (apply_flags_reversibly(flags) < 0) {
			return -1;
		}

		int meta_result = run_meta(meta_cmd, args, &meta_out);
		fflush(stdout);
		fflush(stderr);

		if (meta_result <= 0) {
			revert_flags(flags);
			return meta_result;
		}

		ast_node *parsed = parse_from_string(meta_out);
		meta_result = eval_ast(parsed);
		ast_recurse_free(parsed);

		context_hist_add(strdup(meta_out));

		free(meta_out);
		revert_flags(flags);
		return meta_result;
	}

	// Builtins
	const builtin *command = search_builtins(*argv0);
	if (command != NULL) {
		if (apply_flags_reversibly(flags) < 0) {
			return -1;
		}

		int res = run_builtin(command, args);
		fflush(stdout);
		fflush(stderr);

		revert_flags(flags);
		return res;
	}

	// Add null-terminator to args
	char *argv[args->count + 1];
	memcpy(argv, args->data, args->count * sizeof(char *));
	argv[args->count] = NULL;

	// Exec program
	return exec_normal(argv, flags);
}
