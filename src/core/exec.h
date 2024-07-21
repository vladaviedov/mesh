#pragma once

/**
 * @brief Exec wrapper for normal processes.
 *
 * @param[in] argv - Child arguments.
 * @return Return code.
 */
int exec_normal(char **argv);

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
