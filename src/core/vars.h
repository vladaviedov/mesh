/**
 * @file core/vars.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Environment variables.
 */
#pragma once

/**
 * @brief Import environment variables into shell.
 *
 * @param[in] env - Imported null-terminated environment list.
 */
void vars_import(const char **env);

/**
 * @brief Export environment variables.
 *
 * @return Null-terminated list of environment vars; NULL on error.
 * @note Allocated return value.
 */
char **vars_export(void);

/**
 * @brief Create or edit an environment variable.
 *
 * @param[in] key - Variable name.
 * @param[in] value - New variable value.
 */
void vars_set(const char *key, const char *value);

/**
 * @brief Get value of an environment variable.
 *
 * @param[in] key - Variable name.
 * @return Variable value; NULL on error.
 */
const char *vars_get(const char *key);

/**
 * @brief Delete environment variable.
 *
 * @param[in] key - Variable name.
 * @return 0 on success, -1 on failure.
 */
int vars_delete(const char *key);

/**
 * @brief Set environment variable as export.
 *
 * @param[in] key - Variable name.
 * @return 0 on success, -1 on failure.
 */
int vars_set_export(const char *key);

/**
 * @brief Print all environment variables.
 *
 * @param[in] export_flag - If set, prefix with export and only print exported
 * vars.
 */
void vars_print_all(int export_flag);
