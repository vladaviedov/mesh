#pragma once

#include <stdint.h>

typedef struct context context;

/**
 * @brief Create new context.
 *
 * @param[in] name - Context name.
 * @param[out] ctx_out - If not NULL, populate with new context.
 * @return 0 on success; -1 on failure.
 */
int context_new(const char *name, context **ctx_out);

/**
 * @brief Select context.
 *
 * @param[in] name - Context name.
 * @return 0 on success; -1 on failure.
 */
int context_select(const char *name);

/**
 * @brief Get row from current context.
 *
 * @param[in] index - Absolute index.
 * @return Command at index; NULL on error.
 */
const char *context_get_row_abs(uint32_t index);

/**
 * @brief Get row from current context.
 *
 * @param[in] index - Relative index.
 * @return Command at index; NULL on error.
 */
const char *context_get_row_rel(uint32_t index);

/**
 * @brief Add new command to the context.
 *
 * @param[in] command - Command to add.
 * @param[in] context - Context to manipulate; if NULL, uses current context.
 * @return 0 on success; -1 on error
 */
int context_add(const char *command, context *ctx);
