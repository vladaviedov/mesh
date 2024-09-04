/**
 * @file core/builtins.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Shell built-ins.
 */
#define _POSIX_C_SOURCE 200809L
#include "builtins.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <c-utils/vector.h>

#include "../util/helper.h"
#include "../util/error.h"
#include "vars.h"

#define CODE_OK 0
#define CODE_GEN_ERROR 1
#define CODE_USAGE_ERROR 2
#define CODE_EXIT_ERROR 128

// Builtin table entry
// Note: typedef in header
struct builtin {
	const char *name;
	int (*const func)(uint32_t argc, char **argv);
};

// Builtin functions
static int shell_exit(uint32_t argc, char **argv);
static int shell_cd(uint32_t argc, char **argv);
static int shell_set(uint32_t argc, char **argv);
static int shell_export(uint32_t argc, char **argv);
static int shell_exec(uint32_t argc, char **argv);

// Builtin registry
static const builtin registry[] = {
	{ .name = "exit", .func = &shell_exit },
	{ .name = "cd", .func = &shell_cd },
	{ .name = "set", .func = &shell_set },
	{ .name = "export", .func = &shell_export },
	{ .name = "exec", .func = &shell_exec },
};
static const size_t registry_length = sizeof(registry) / sizeof(builtin);

int pure_assign(uint32_t count, char **args, int export_flag) {
	for (uint32_t i = 0; i < count; i++) {
		char *key = strtok(args[i], "=");
		char *value = strtok(NULL, "=");

		vars_set(key, value);
		if (export_flag) {
			if (vars_set_export(key) < 0) {
				return -1;
			}
		}
	}

	return 0;
}

const builtin *search_builtins(const char *name) {
	for (size_t i = 0; i < registry_length; i++) {
		if (strcmp(registry[i].name, name) == 0) {
			return registry + i;
		}
	}

	return NULL;
}

int run_builtin(const builtin *cmd, string_vector *args) {
	return cmd->func(args->count, args->data);
}

/** Builtin implementations */

static int shell_exit(uint32_t argc, char **argv) {
	if (argc > 2) {
		print_error("exit: too many arguments\n");
		return CODE_USAGE_ERROR;
	}

	if (argc == 2) {
		char *end;
		int code = strtol(argv[1], &end, 10);

		// If string is not only numbers
		if (*end != '\0') {
			print_error("exit: invalid exit code '%s'\n", argv[1]);
			return CODE_EXIT_ERROR;
		}

		exit(code);
	}

	exit(EXIT_SUCCESS);
}

static int shell_cd(uint32_t argc, char **argv) {
	if (argc > 2) {
		print_error("cd: too many arguments\n");
		return CODE_USAGE_ERROR;
	}

	// Get target path
	const char *path = (argc == 1) ? vars_get("HOME") : argv[1];
	if (strcmp(path, "-") == 0) {
		if ((path = vars_get("OLDPWD")) == NULL) {
			print_error("cd: nowhere to go\n");
			return CODE_GEN_ERROR;
		}

		printf("%s\n", path);
	}

	if (chdir(path) < 0) {
		print_error("cd: %s: %s\n", path, strerror(errno));
		return CODE_GEN_ERROR;
	}

	// Update vars
	vars_set("OLDPWD", vars_get("PWD"));
	char *pwd = getcwd(NULL, 0);
	vars_set("PWD", pwd);
	free(pwd);

	return CODE_OK;
}

static int shell_set(uint32_t argc, char **argv) {
	if (argc > 1) {
		// TODO: implement
		print_error("set: this function is not implemented");
		return CODE_USAGE_ERROR;
	}

	vars_print_all(0);
	return CODE_OK;
}

static int shell_export(uint32_t argc, char **argv) {
	if (argc == 1) {
		vars_print_all(1);
		return CODE_OK;
	}

	pure_assign(argc - 1, argv + 1, 1);
	return CODE_OK;
}

static int shell_exec(uint32_t argc, char **argv) {
	if (argc == 1) {
		return CODE_OK;
	}

	extern char **environ;
	environ = vars_export();

	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);

	// First argument is skipped
	char **exec_argv = argv + 1;
	execvp(exec_argv[0], exec_argv);

	print_error("exec: %s: command not found\n", exec_argv[0]);
	return CODE_GEN_ERROR;
}
