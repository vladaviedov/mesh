/**
 * @file ext/context.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Mesh context manipulation.
 */
#pragma once

#include <stdint.h>

#include <c-utils/vector.h>

typedef struct context {
	char *name;
	vector commands;
} context;

typedef vector context_vector;

/**
 * @brief Create new context.
 *
 * @param[in] name - Context name.
 * @param[out] ctx_out - If not NULL, populate with new context.
 * @return 0 on success; -1 on failure.
 */
int context_new(const char *name, context **ctx_out);

/**
 * @brief Delete context.
 *
 * @param[in] name - Context name.
 * @return 0 on success; -1 on failure.
 */
int context_delete(const char *name);

/**
 * @brief Select context.
 *
 * @param[in] name - Context name.
 * @return 0 on success; -1 on failure.
 */
int context_select(const char *name);

/**
 * @brief Get current context.
 *
 * @param[in] name - Context name; if NULL get current.
 * @return Context object; NULL if not set.
 */
const context *context_get(const char *name);

/**
 * @brief Get all contexts.
 *
 * @return Context vector; NULL if not initialized.
 */
const vector *context_get_all(void);

/**
 * @brief Get row from current context.
 *
 * @param[in] index - Index; positive is absolute, negative is relative.
 * @return Command at index; NULL on error.
 */
const char *context_get_row(int32_t index);

/**
 * @brief Add new command to the context.
 *
 * @param[in] command - Command to add.
 * @param[in] context - Context to manipulate; if NULL, uses current context.
 * @return 0 on success; -1 on error
 */
int context_add(const char *command, context *ctx);

/**
 * @brief Create the history context.
 */
int context_hist_init(void);

/**
 * @brief Add new command to history.
 *
 * @param[in] command - Command to add.
 * @return 0 on success; -1 on error
 */
int context_hist_add(const char *command);
