#include "lexer.h"

#include <ctype.h>
#include <stdint.h>

#include "../util/parsing.h"
#include "../util/vector.h"

void push_non_empty(token_vec *vec, token *maybe_token);
token parse_redir(const char *hit, uint32_t before_count);
token parse_logic_or_pipe(const char *hit);

token_vec *lexer_run(const char *input) {
	token_vec *tokens = vec_new(sizeof(token_vec));

	char ch;
	quote_st quotes = QUOTE_NONE;
	uint32_t length = 0;
	
	while ((ch = *input) != '\0') {
		// Parse other special chars
		switch (ch) {
			case '\\':
				input++;
				length++;
				break;
			case '\'':
				switch (quotes) {
					case QUOTE_NONE:
						quotes = QUOTE_SINGLE;
						break;
					case QUOTE_SINGLE:
						quotes = QUOTE_NONE;
						break;
					case QUOTE_DOUBLE:
						break;
				}
				break;
			case '\"':
				switch (quotes) {
					case QUOTE_NONE:
						quotes = QUOTE_DOUBLE;
						break;
					case QUOTE_DOUBLE:
						quotes = QUOTE_NONE;
						break;
					case QUOTE_SINGLE:
						break;
				}
				break;
			case '<': // fallthrough
			case '>': {
				token redir = parse_redir(input, length);

				// If redir starts eariler, adjust length of statement
				uint32_t length_adj = input - redir.start;

				// Create statement token and push tokens
				token stat = {
					.type = STATEMENT,
					.start = input - length,
					.length = length - length_adj
				};

				push_non_empty(tokens, &stat);
				push_non_empty(tokens, &redir);

				// Set new token start where redir ends
				input = redir.start + redir.length;
				length = 0;
				continue;
			}
			case ';': // fallthrough
			case '&': // fallthrough
			case '|': {
				token logic_or_pipe = parse_logic_or_pipe(input);

				// Create statement
				token stat = {
					.type = STATEMENT,
					.start = input - length,
					.length = length
				};

				push_non_empty(tokens, &stat);
				push_non_empty(tokens, &logic_or_pipe);

				input = logic_or_pipe.start + logic_or_pipe.length;
				length = 0;
				continue;
			}
			default:
				break;
		}

		length++;
		input++;
	}

	// Create last token
	token last = {
		.type = STATEMENT,
		.start = input - length,
		.length = length
	};
	push_non_empty(tokens, &last);

	return tokens;
}

int lexer_verify(const token_vec *tokens) {
	// First token - cannot be logical
	token_type last_type = ((token *)vec_at(tokens, 0))->type;
	if (last_type == LOGICAL) {
		return -1;
	}

	for (uint32_t i = 1; i < tokens->count; i++) {
		token_type type = ((token *)vec_at(tokens, i))->type;

		switch (last_type) {
			case INTERNAL_REDIR:
				// Self-contained
				break;
			case STATEMENT:
				// Cannot have two statements next to each other
				if (type == STATEMENT) {
					return -1;
				}
				break;
			case PIPE: // fallthrough
			case REDIR:
				// Must be followed with a statement
				if (type != STATEMENT) {
					return -1;
				}
				break;
			case LOGICAL:
				// or internal redir here
				if (type != STATEMENT && type != INTERNAL_REDIR) {
					return -1;
				}
				break;
		}

		last_type = type;
	}

	// Last token - cannot be external redir
	if (last_type == REDIR) {
		return -1;
	}

	return 0;
}

/**
 * @brief Push token only if it has non-blank characters.
 *
 * @param[in/out] vec - Vector object.
 * @param[in] maybe_token - Token tested.
 */
void push_non_empty(token_vec *vec, token *maybe_token) {
	for (uint32_t i = 0; i < maybe_token->length; i++) {
		if (!is_blank(maybe_token->start[i])) {
			vec_push(vec, maybe_token);
			return;
		}
	}
}

/**
 * @brief Parse and size (internal) redirection.
 *
 * @param[in] hit - Location of the first angle bracket.
 * @param[in] before_count - Characters available to read before the hit.
 * @return (Internal) Redirection token.
 */
token parse_redir(const char *hit, uint32_t before_count) {
	// All the expressions we need to check:
	// REDIR:
	// n<
	// n<<
	// n<<-
	// n<>
	// n>
	// n>|
	// INTERNAL_REDIR:
	// n>&m
	// n>&-
	// n<&m
	// n<&-

	token t = {
		.type = REDIR,
		.start = hit,
		.length = 1
	};
	
	// Check for 'n' - go back until first digit
	const char *ptr = hit;
	while (before_count > 0 && isdigit(*(ptr - 1))) {
		ptr--;
		before_count--;
	}

	// Only add the number if it's not connected to a word
	if (before_count == 0 || is_blank(*(ptr - 1))) {
		t.start = ptr;
		t.length += hit - ptr;
	}

	// Look after hit
	ptr = hit + 1;
	
	// Check for internal redirect
	if (*ptr++ == '&') {
		t.type = INTERNAL_REDIR;
		t.length++;

		// '-' or number
		if (*ptr == '-') {
			t.length++;
		} else {
			while (isdigit(*ptr)) {
				t.length++;
				ptr++;
			}
		}

		return t;
	}

	// Check for character-specific
	ptr = hit + 1;
	if (*hit == '<') {
		if (*ptr == '<') {
			t.length++;

			if (*(ptr + 1) == '-') {
				t.length++;
			}
		}

		if (*ptr == '>') {
			t.length++;
		}
	} else {
		if (*ptr == '>' || *ptr == '|') {
			t.length++;
		}
	}
	
	return t;
}

/**
 * @brief Parse and size logical token.
 *
 * @param[in] hit - Location of the first character.
 * @param[in] before_count - Characters available to read before the hit.
 * @return Logical token.
 */
token parse_logic_or_pipe(const char *hit) {
	// All operations we need to check:
	// ;
	// &
	// &&
	// |
	// ||

	token t = {
		.type = LOGICAL,
		.start = hit,
		.length = 1
	};

	if (*hit == ';') {
		return t;
	}

	char next = *(hit + 1);
	if (*hit == '&') {
		if (next == '&') {
			t.length++;
		}

		return t;
	}

	// *hit == '|'
	if (next == '|') {
		t.length++;
	} else {
		t.type = PIPE;
	}

	return t;
}
