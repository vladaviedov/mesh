#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include <c-utils/vector.h>

#include "grammar/ast.h"
#include "util/helper.h"
#include "core/vars.h"
#include "core/run.h"
#include "core/scope.h"
#include "core/eval.h"
#include "ext/context.h"
#include "ext/meta.h"
#include "grammar/parse.h"
#include "grammar/expand.h"

#define PS1_ROOT "# "
#define PS1_USER "$ "

#define PID_STR_MAX_LEN 32

void set_vars(void);
void run_from_stream(FILE *stream);
void run_script(const char *filename);
int process_cmd(char *buffer);

int main(int argc, char **argv) {
	set_vars();
	scope_init();

	context_hist_init();
	context_select("history");

	if (argc > 1) {
		if (argv[1][0] == '-') {
			if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
				printf("mesh version 0.2.0\n");
				return 0;
			}
			if (strcmp(argv[1], "-c") == 0) {
				if (argc > 2) {
					// Get positional arguments
					for (int i = 3; i < argc; i++) {
						scope_append_pos(argv[i]);
					}

					return process_cmd(argv[2]);
				} else {
					print_error("'-c': requires an argument\n");
					return 1;
				}
			}

			print_error("invalid argument\n");
			return 1;
		}

		// Get positional arguments
		for (int i = 2; i < argc; i++) {
			scope_append_pos(argv[i]);
		}

		run_script(argv[1]);
		return 1;
	}

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	while (1) {
		printf("%s", vars_get("PS1"));
		run_from_stream(stdin);
	}

	return 0;
}

/**
 * @brief Load environment and set shell variables.
 */
void set_vars(void) {
	// Load environment
	extern const char **environ;
	vars_import(environ);

	// Prompts
	if (vars_get("PS1") == NULL) {
		char *ps1 = (getuid() == 0) ? PS1_ROOT : PS1_USER;
		vars_set("PS1", ps1);
	}

	// PID
	pid_t pid = getpid();
	char pid_str[PID_STR_MAX_LEN];
	snprintf(pid_str, PID_STR_MAX_LEN, "%d", pid);
	vars_set("$", pid_str);
	
	// PWD
	char *pwd = getcwd(NULL, 0);
	vars_set("PWD", pwd);
	free(pwd);
}

/**
 * @brief Run shell commands read from a specified stream.
 */
void run_from_stream(FILE *stream) {
	int last_result = 0;
	char buffer[1024];
	uint32_t index = 0;

	memset(buffer, 0, 1024);
	index = 0;

	int ch;
	while ((ch = fgetc(stream)) != '\n' && ch != EOF) {
		buffer[index] = ch;
		index++;
	}

	if (ch == EOF && errno != EINTR) {
		// Clear line on EOF in interactive mode
		if (stream == stdin) {
			putchar('\n');
		}

		exit(last_result);
	}

	// Execute
	last_result = process_cmd(buffer);

	// Return code variable
	char var_result[16];
	snprintf(var_result, 16, "%d", last_result);
	vars_set("?", var_result);
}

/**
 * @brief Run shell script file.
 *
 * @param[in] filename - Script filename.
 */
void run_script(const char *filename) {
	FILE *script = fopen(filename, "r");
	if (!script) {
		print_error("failed to open file: %s\n", strerror(errno));	
		return;
	}

	// Will exit
	while (1) {
		run_from_stream(script);
	}
}

/**
 * @brief Process inputted command.
 *
 * @param[in] buffer - Raw command.
 * @return Status code.
 */
int process_cmd(char *buffer) {
	ast_node *root = parse_from_string(buffer);
	int result = eval_ast(root);
	ast_recurse_free(root);

	if (buffer[0] != ':') {
		context_hist_add(strdup(buffer));
	}

	return result;
}
