/**
 * @file grammar/expand.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief Word expansions.
 */
#pragma once

/**
 * @brief Perform all expansions.
 *
 * @param[in] word - Input string.
 * @return Expanded string.
 */
char *expand_word(const char *word);

/**
 * @brief Make changes to the buffer before the parser is run.
 *
 * @param[in] input - Input buffer.
 * @param[out] output - Output buffer.
 * @return 0 on success; -1 on error.
 * @note Allocated value placed in output.
 */
int preprocess_buffer(const char *input, char **output);
