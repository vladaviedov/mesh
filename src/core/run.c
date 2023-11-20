#include "run.h"

// TEMP
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "../util/helper.h"
#include "../ext/meta.h"
#include "vars.h"
#include "parser.h"
#include "builtins.h"

int exec_prog(str_vec *args);

int run_dispatch(str_vec *args) {
	// Meta commands
	char *argv0 = fix_ptr(vec_at(args, 0));
	if (*argv0 == ':') {
		char *meta_out;
		int meta_result = run_meta(args, &meta_out);

		if (meta_result < 0) {
			return -1;
		}
		if (meta_result == 0) {
			return 0;
		}

		char *subbed_str = parser_sub(meta_out);

		char *end = subbed_str;
		int result;
		do {
			str_vec *parsed_str = parser_split(end, &end);
			result = run_dispatch(parsed_str);
			vec_free_with_elements(parsed_str);
		} while (end != NULL);

		free(subbed_str);
		free(meta_out);
		return result;
	}

	// Shell assignments
	if (is_pure_assign(args)) {
		return pure_assign(args->count, args->raw, 0);
	}
	
	// Builtins
	int code = run_builtin(args);
	if (code >= 0) {
		return code;
	}
	
	// Exec program
	return exec_prog(args);
}

/**
 * @brief Run regular program.
 *
 * @param[in] args - Argument vector.
 * @return Return code.
 */
int exec_prog(str_vec *args) {
	pid_t pid = fork();

	if (pid < 0) {
		// Fork failed
		// TODO: fork failed error msg
		return -1;
	} else if (pid == 0) {
		// Child - load environment and exec
		extern char **environ;
		environ = vars_export();

		// TOOD: proper signal reset
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);

		char **argv = args->raw;
		execvp(argv[0], argv);

		// Exec failed
		print_error("%s: command not found\n", argv[0]);
		exit(1);
	} else {
		// Parent - wait
		int result;
		wait(&result);
		return WEXITSTATUS(result);
	}
}
