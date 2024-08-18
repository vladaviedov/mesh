/**
 * @file main.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <c-utils/nanorl.h>
#include <c-utils/vector.h>

#include "core/eval.h"
#include "core/run.h"
#include "core/scope.h"
#include "core/vars.h"
#include "ext/context.h"
#include "ext/meta.h"
#include "grammar/ast.h"
#include "grammar/expand.h"
#include "grammar/parse.h"
#include "util/helper.h"

#define PS1_ROOT "# "
#define PS1_USER "$ "

#define PID_STR_MAX_LEN 32

#ifndef MESH_VERSION
#define MESH_VERSION "0.2.0"
#endif

void run_from_stream(FILE *stream);
static void set_vars(void);
static void run_script(const char *filename);
static int process_cmd(char *buffer);

int main(int argc, char **argv) {
	set_argv0((const char *const *)argv);

	set_vars();
	scope_init();

	context_hist_init();
	context_select("history");

	if (argc > 1) {
		if (argv[1][0] == '-') {
			if (strcmp(argv[1], "--version") == 0
				|| strcmp(argv[1], "-v") == 0) {
				printf("mesh version %s\n", MESH_VERSION);
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
		run_from_stream(stdin);
	}

	return 0;
}

/**
 * @brief Run shell commands read from a specified stream.
 *
 * @param[in] stream - File stream.
 */
void run_from_stream(FILE *stream) {
	int last_result = 0;

	nrl_error err;
	char *input = nanorl(vars_get("PS1"), &err);
	switch (err) {
	case NRL_ERR_BAD_FD:
		print_error("cannot read commands from this source\n");
		exit(1);
	case NRL_ERR_SYS:
		if (stream == stdin) {
			putchar('\n');
		}

		exit(0);
	case NRL_ERR_EMPTY:
		break;
	case NRL_ERR_OK:
		// Hacky fix for nanorl 1.0 design flaw
		// Will be revorked in later versions
		if (errno == EAGAIN || errno == EINTR) {
			errno = 0;
			last_result = 2;
		} else {
			last_result = process_cmd(input);
		}
		break;
	}

	// Return code variable
	char var_result[16];
	snprintf(var_result, 16, "%d", last_result);
	vars_set("?", var_result);
	free(input);
}

/**
 * @brief Load environment and set shell variables.
 */
static void set_vars(void) {
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
 * @brief Run shell script file.
 *
 * @param[in] filename - Script filename.
 */
static void run_script(const char *filename) {
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
static int process_cmd(char *buffer) {
	char *processed;
	if (preprocess_buffer(buffer, &processed) < 0) {
		return 1;
	}

	ast_node *root = parse_from_string(processed);
	if (root == NULL) {
		free(processed);
		return 1;
	}

	int result = eval_ast(root);
	ast_recurse_free(root);

	if (processed[0] != ':') {
		context_hist_add(processed);
	} else {
		free(processed);
	}

	return result;
}
