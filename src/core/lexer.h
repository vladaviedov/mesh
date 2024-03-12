#pragma once

#include <stdint.h>

#include "../util/vector.h"

typedef enum {
	STATEMENT,
	REDIR,
	LOGICAL
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
