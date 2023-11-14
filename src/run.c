#include "run.h"

// TEMP
#include <stdio.h>

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "vars.h"

int exec_prog(str_vec *args);

int run_dispatch(str_vec *args) {
	// TODO: run pure assignment
	
	// TODO: run built-in commands
	
	// TODO: meta commands
	
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
		printf("command not found\n");
		exit(1);
	} else {
		// Parent - wait
		int result;
		wait(&result);
		return WEXITSTATUS(result);
	}
}
