#include "exec.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../util/helper.h"
#include "vars.h"

// Forward declare from main.c
void run_from_stream(const FILE *stream);

int exec_normal(char **argv) {
	pid_t pid = fork();

	if (pid < 0) {
		// Fork failed
		print_error("failed to create new process\n");
		return -1;
	} else if (pid == 0) {
		// Child - load environment and exec
		extern char **environ;
		environ = vars_export();

		// TODO: proper signal reset
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);

		// Execute
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

int exec_silent(char **argv) {
	pid_t pid = fork();

	if (pid < 0) {
		// Fork failed
		print_error("failed to create new process\n");
		return -1;
	} else if (pid == 0) {
		// Child - load environment and exec
		extern char **environ;
		environ = vars_export();

		// TODO: proper signal reset
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);

		// Redirect stdout to /dev/null
		int null_fd = open("/dev/null", 0);
		close(STDOUT_FILENO);
		dup2(null_fd, STDOUT_FILENO);

		// Execute
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

int exec_subshell(char *cmd, int fd_pipe_out) {
	pid_t pid = fork();

	if (pid < 0) {
		// Fork failed
		print_error("failed to create new process\n");
		return -1;
	} else if (pid == 0) {
		// Redirect stdin to pipe
		int stdin_pipe[2];
		if (pipe(stdin_pipe) < 0) {
			return -1;
		}
		close(STDIN_FILENO);
		dup2(stdin_pipe[0], STDIN_FILENO);

		// Write command to stdin
		write(stdin_pipe[1], cmd, strlen(cmd));

		// Redirect stdout to pipe
		close(STDOUT_FILENO);
		dup2(fd_pipe_out, STDOUT_FILENO);

		run_from_stream(stdin);
		exit(0);
	} else {
		// Parent - wait
		int result;
		wait(&result);
		return WEXITSTATUS(result);
	}
}
