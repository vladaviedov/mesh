#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include "util/helper.h"
#include "util/vector.h"
#include "core/vars.h"
#include "core/parser.h"
#include "core/run.h"
#include "ext/context.h"
#include "ext/meta.h"

#define PS1_ROOT "# "
#define PS1_USER "$ "

#define PID_STR_MAX_LEN 32

void set_vars(void);
void interactive(void);
int process_cmd(char *buffer);

static context *shell_ctx;

int main(int argc, char **argv) {
	if (argc > 1) {
		if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
			printf("mesh version 0.1.0\n");
			return 0;
		}

		print_error("invalid argument\n");
		return 1;
	}

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	set_vars();

	context_new("SHELL", &shell_ctx);
	context_select("SHELL");

	while (1) {
		interactive();
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
 * @brief Run shell in interactive mode.
 */
void interactive(void) {
	char buffer[1024];
	uint32_t index = 0;

	memset(buffer, 0, 1024);
	index = 0;

	printf("%s", vars_get("PS1"));
	int ch;
	while ((ch = fgetc(stdin)) != '\n' && ch != EOF) {
		buffer[index] = ch;
		index++;
	}

	if (ch == EOF && errno != EINTR) {
		putchar('\n');
		exit(0);
	}

	if (index == 0) {
		return;
	}
	if (buffer[0] != ':') {
		context_add(strdup(buffer), shell_ctx);
	}

	// Return code variable
	char var_result[16];
	snprintf(var_result, 16, "%d", process_cmd(buffer));
	vars_set("?", var_result);
}

/**
 * @brief Process inputted command.
 *
 * @param[in] buffer - Raw command.
 * @return Status code.
 */
int process_cmd(char *buffer) {
	char *subbed_str = parser_sub(buffer);

	char *end = subbed_str;
	int result;
	do {
		str_vec *parsed_str = parser_split(end, &end);
		result = run_dispatch(parsed_str);
		vec_free_with_elements(parsed_str);
	} while (end != NULL);

	free(subbed_str);
	return result;
}
