#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

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

int main() {
	extern const char **environ;
	vars_import(environ);
	set_vars();

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	context *shell_ctx;
	context_new("SHELL", &shell_ctx);
	context_select("SHELL");

	char buffer[1024];
	uint32_t index = 0;

	while (1) {
		memset(buffer, 0, 1024);
		index = 0;

		printf("%s", vars_get("PS1"));
		int ch;
		while ((ch = fgetc(stdin)) != '\n' && ch != EOF) {
			buffer[index] = ch;
			index++;
		}

		if (buffer[0] != ':') {
			context_add(strdup(buffer), shell_ctx);
		}

		char *subbed_str = parser_sub(buffer);
		str_vec *parsed_str = parser_split(subbed_str);

		run_dispatch(parsed_str);
		vec_free_with_elements(parsed_str);
		free(subbed_str);
	}

	return 0;
}

void set_vars(void) {
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
