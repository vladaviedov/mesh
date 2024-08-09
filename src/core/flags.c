/**
 * @file core/flags.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief Apply redirections and assignments.
 */
#define _POSIX_C_SOURCE 200809L
#include "flags.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../util/helper.h"
#include "scope.h"
#include "vars.h"

static void partial_revert_redirs(run_flags *flags, uint32_t stop_index);
static int redirect(const redir *op);

run_flags copy_flags(const run_flags *flags) {
	run_flags new_flags = {
		.redirs = vec_init_clone(&flags->redirs),
		.assigns = vec_init_clone(&flags->assigns),
	};

	return new_flags;
}

void del_flags(run_flags *flags) {
	vec_deinit(&flags->redirs);
	vec_deinit(&flags->assigns);
}

int apply_flags(const run_flags *flags) {
	// Perform redirections
	for (uint32_t i = 0; i < flags->redirs.count; i++) {
		const redir *op = vec_at(&flags->redirs, i);

		if (redirect(op) < 0) {
			return -1;
		}
	}

	// Perform assignments
	for (uint32_t i = 0; i < flags->assigns.count; i++) {
		const assign *op = vec_at(&flags->assigns, i);
		vars_set(op->key, op->value);
		vars_set_export(op->key);
	}

	return 0;
}

int apply_flags_reversibly(run_flags *flags) {
	// Revert will always delete a stack frame and we might need to exit early
	scope_create_frame();

	// Perform redirections
	for (uint32_t i = 0; i < flags->redirs.count; i++) {
		redir *op = vec_at_mut(&flags->redirs, i);

		int backup = dup(op->from);
		if (backup < 0 && errno != EBADF) {
			partial_revert_redirs(flags, i);
			return -1;
		}

		// Set O_CLOEXEC (to close fds on fatal error)
		int fd_flags = fcntl(backup, F_GETFD);
		fcntl(backup, F_SETFD, fd_flags | FD_CLOEXEC);
		if (op->type == RDR_FILE) {
			op->flags |= O_CLOEXEC;
		}

		if (redirect(op) < 0) {
			partial_revert_redirs(flags, i);
			return -1;
		}

		if (backup < 0) {
			// If not open before, close it after we're done
			op->type = RDR_CLOSE;
		} else {
			// If it was open, redirect it back
			op->type = RDR_FD;
			op->to.fd = backup;
		}
	}

	// Perform assignments
	for (uint32_t i = 0; i < flags->assigns.count; i++) {
		const assign *op = vec_at(&flags->assigns, i);
		scope_set_var(op->key, op->value);
	}

	return 0;
}

void revert_flags(const run_flags *flags) {
	// Revert assignments
	scope_delete_frame();

	// Revert redirections
	if (apply_flags(flags) < 0) {
		print_fatal_hcf("failed to revert redirections\n");
	}

	// Close backup file descriptors
	for (uint32_t i = 0; i < flags->redirs.count; i++) {
		const redir *op = vec_at(&flags->redirs, i);

		if (op->type == RDR_FD) {
			close(op->to.fd);
		}
	}
}

/**
 * @brief Perform one redirection.
 *
 * @param[in] op - Redirection operation.
 * @return 0 on success; -1 on error.
 */
static int redirect(const redir *op) {
	switch (op->type) {
	case RDR_FD:
		if (dup2(op->to.fd, op->from) < 0) {
			return -1;
		}
		break;
	case RDR_FILE: {
		int file_fd = open(op->to.filename, op->flags, 0644);
		if (file_fd < 0) {
			return -1;
		}

		int res = dup2(file_fd, op->from);
		close(file_fd);
		if (res < 0) {
			return -1;
		}
		break;
	}
	case RDR_CLOSE:
		close(op->from);
		break;
	}

	return 0;
}

/**
 * @brief Revert flags before a given index in the vector.
 *
 * @param[in] flags - Partial backup flag data.
 * @param[in] stop_index - Finish index (exclusive).
 */
static void partial_revert_redirs(run_flags *flags, uint32_t stop_index) {
	// Fake count for the operation to end early
	uint32_t count = flags->redirs.count;
	flags->redirs.count = stop_index;

	revert_flags(flags);

	// Restore count
	flags->redirs.count = count;
}
