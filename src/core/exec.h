/**
 * @file core/exec.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Execute external programs with fork.
 */
#pragma once

#include "run.h"

/**
 * @brief Exec wrapper for normal processes.
 *
 * @param[in] argv - Child arguments.
 * @param[in] flags - Special run flags.
 * @return Return code.
 */
int exec_normal(char **argv, const run_flags *flags);

/**
 * @brief Exec wrapper with silenced output.
 *
 * @param[in] argv - Child arguments.
 * @return Return code.
 */
int exec_silent(char **argv);

/**
 * @brief Command to run in a subshell.
 *
 * @param[in] cmd - Command sent to stdin.
 * @param[in] fd_pipe_out - Pipe to replace stdout with.
 * @return Return code.
 */
int exec_subshell(const char *cmd, int fd_pipe_out);
