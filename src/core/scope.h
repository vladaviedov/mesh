#pragma once

#include <stdint.h>

/**
 * @brief Initialize top-level scope.
 */
void scope_init(void);

/**
 * @brief Set scoped variable value.
 *
 * @param[in] key - Variable name.
 * @param[in] value - New variable value.
 */
void scope_set_var(const char *key, const char *value);

/**
 * @brief Get scoped variable value.
 *
 * @param[in] key - Variable name.
 * @return Variable value; NULL on error.
 */
const char *scope_get_var(const char *key);

/**
 * @brief Delete scoped variable.
 *
 * @param[in] key - Variable name.
 * @return 0 on success; -1 on failure.
 */
int scope_delete_var(const char *key);

/**
 * @brief Get positional argument count.
 *
 * @return Variable count.
 */
uint32_t scope_pos_count(void);

/**
 * @brief Append positional variable.
 *
 * @param[in] value - New variable value.
 */
void scope_append_pos(const char *value);

/**
 * @brief Get positional variable value.
 *
 * @param[in] index - Positional index.
 * @return Variable value; NULL on error.
 */
const char *scope_get_pos(uint32_t index);

/**
 * @brief Get all positional arguments.
 *
 * @return Positional variables as a string separated by spaces.
 */
char *scope_list_pos(void);

/**
 * @brief Reset positional arguments in current scope.
 */
void scope_reset_pos(void);

/**
 * @brief Create new scope.
 */
void scope_create_frame(void);

/**
 * @brief Delete current scope and return to previous one.
 *
 * @return 0 on success; -1 on failure.
 */
int scope_delete_frame(void);
