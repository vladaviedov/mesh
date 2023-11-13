#pragma once

#include "vector.h"

/**
 * @brief Split input string into tokens.
 *
 * @param[in] input_string - Input string.
 * @return Split string vector.
 * @note Allocated return value.
 */
str_vec *parser_split(char *input_string);

/**
 * @brief Do shell substitutions and generate a new string.
 *
 * @param[in] input_string - Input string.
 * @return New string with substitutions.
 * @note Allocated return value.
 */
char *parser_sub(char *input_string);
