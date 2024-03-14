#pragma once

#include <stdint.h>

#include "../util/vector.h"

typedef enum {
	STATEMENT,
	REDIR,
	INTERNAL_REDIR,
	LOGICAL,
	PIPE
} token_type;

typedef struct {
	token_type type;
	const char *start;
	uint32_t length;
} token;

typedef vector token_vec;

/**
 * @brief Break inputted command into lexical parts.
 *
 * @param[in] input - Input string.
 * @return Token vector.
 * @note Allocated return value.
 */
token_vec *lexer_run(const char *input);

/**
 * @brief Verify that the entire command is lexically sound.
 *
 * @param[in] tokens - Token vector.
 * @return 0 if passed, -1 is did not.
 */
int lexer_verify(const token_vec *tokens);
