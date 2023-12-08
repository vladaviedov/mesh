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
 * @brief Exec wrapper with stdout redirection.
 *
 * @param[in] argv - Child arguments.
 * @param[in] pipe_fd - Stdout pipe file descriptor.
 * @return Return code.
 */
int exec_stdout_pipe(char **argv, int pipe_fd);
