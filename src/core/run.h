#pragma once

#include <c-utils/vector.h>

#include "../util/helper.h"

typedef struct {
	int fd_from;
	int fd_to;
} redir;
typedef vector redir_vector;

typedef struct {
	char *key;
	char *value;
} assign;
typedef vector assign_vector;

typedef struct {
	redir_vector redirs;
	assign_vector assigns;
} run_flags;

/**
 * @brief Determines how to run a command and runs it.
 *
 * @param[in] args - Processed argument vector.
 * @param[in] flags - Flags for this execution.
 * @return Status code.
 */
int run_dispatch(string_vector *args, run_flags *flags);
