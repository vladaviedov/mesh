/**
 * @file core/exec.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Execute external programs with fork.
 */
#define _POSIX_C_SOURCE 200809L
#include "exec.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../util/helper.h"
#include "run.h"
#include "vars.h"

// from main.c
extern void run_from_stream(const FILE *stream);

static int do_redirs(const redir_vector *redirs);
static void do_assigns(const assign_vector *assigns);

int exec_normal(char **argv, const run_flags *flags) {
	pid_t pid = fork();

	if (pid < 0) {
		// Fork failed
		print_error("failed to create new process\n");
		return -1;
	} else if (pid == 0) {
		// Child - prepare and exec
		if (do_redirs(&flags->redirs) < 0) {
			print_error("failed to perform redirections\n");
			exit(1);
		}
		do_assigns(&flags->assigns);

		// Load environment
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

int exec_subshell(const char *cmd, int fd_pipe_out) {
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

/**
 * @brief Perform all redirections.
 *
 * @param[in] redirs - Redirections.
 * @return 0 on success, -1 on error.
 * @note Should be run in the child thread.
 */
static int do_redirs(const redir_vector *redirs) {
	for (uint32_t i = 0; i < redirs->count; i++) {
		const redir *op = vec_at(redirs, i);
		switch (op->type) {
		case RDR_FD:
			if (dup2(op->to.fd, op->from) < 0) {
				print_error("dup failed\n");
				return -1;
			}
			break;
		case RDR_FILE: {
			int file_fd = open(op->to.filename, op->flags, 0644);
			if (file_fd < 0) {
				print_error(
					"open failed on %s: %s\n", op->to.filename, strerror(errno));
				return -1;
			}
			if (dup2(file_fd, op->from) < 0) {
				print_error("dup failed\n");
				return -1;
			}
			break;
		}
		case RDR_CLOSE:
			close(op->from);
			break;
		}
	}

	return 0;
}

/**
 * @brief Perform all assignments.
 *
 * @param[in] assigns - Assignments.
 * @note Should be run in the child thread.
 */
static void do_assigns(const assign_vector *assigns) {
	for (uint32_t i = 0; i < assigns->count; i++) {
		const assign *op = vec_at(assigns, i);
		vars_set(op->key, op->value);
		vars_set_export(op->key);
	}
}
