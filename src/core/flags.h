/**
 * @file core/flags.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief Apply redirections and assignments.
 */
#pragma once

#include <c-utils/vector.h>

typedef enum {
	RDR_FD,
	RDR_FILE,
	RDR_CLOSE,
} redir_type;

typedef union {
	int fd;
	char *filename;
} redir_src;

typedef struct {
	redir_type type;
	int flags;

	int from;
	redir_src to;
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
 * @brief Copy all flags into a new structure.
 *
 * @param[in] flags - Original flags.
 * @return New flag structure.
 */
run_flags copy_flags(const run_flags *flags);

/**
 * @brief Delete flag vectors.
 *
 * @param[in] flags - Flags object.
 */
void del_flags(run_flags *flags);

/**
 * @brief Apply flags in an irreversible way (for forked commands).
 *
 * @param[in] flags - Flags to apply.
 * @return 0 on success; -1 on error.
 */
int apply_flags(const run_flags *flags);

/**
 * @brief Apply flags in a reversible way (for internal commands).
 *
 * @param[in,out] flags - Flags to apply; replaced with backup flag data.
 * @return 0 on success; -1 on error.
 */
int apply_flags_reversibly(run_flags *flags);

/**
 * @brief Revert flags from backup data.
 *
 * @param[in] flags - Backup flag data.
 * @return 0 on success; -1 on failure.
 */
int revert_flags(const run_flags *flags);
