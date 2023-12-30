#pragma once

#include "../util/vector.h"

/**
 * @brief Do shell substitutions and generate a new string.
 *
 * @param[in] input_string - Input string.
 * @return New string with substitutions.
 * @note Allocated return value; NULL if empty.
 */
char *parser_sub(char *input_string);

/**
 * @brief Split input string into tokens.
 *
 * @param[in] input_string - Input string.
 * @param[out] end - Pointer to where first command ends.
 * @return Split string vector.
 * @note Allocated return value.
 */
str_vec *parser_split(char *input_string, char **end);

/**
 * @brief Check if name is a valid variable name.
 *
 * @param[in] name - Potential variable name.
 * @return True/False.
 */
int is_valid_var_name(const char *name);

/**
 * @brief Check if token is an shell assignment operation.
 *
 * @param[in] token - Separated token.
 * @return True/False.
 */
int is_assign(const char *token);

/**
 * @brief Check if expression only contains assignments.
 *
 * @param[in] expression - Argument vector.
 * @return True/False.
 */
int is_pure_assign(const str_vec *expression);
