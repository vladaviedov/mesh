#include "lexer.h"

#include <ctype.h>
#include <stdint.h>
#include <stddef.h>

#include "../util/parsing.h"
#include "../util/vector.h"

token parse_redir(const char *hit, uint32_t before_count);

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

				vec_push(tokens, &stat);
				vec_push(tokens, &redir);

				// Set new token start where redir ends
				input = redir.start + redir.length;
				length = 0;
				continue;
			}
			default:
				break;
		}

		length++;
		input++;
	}

	return tokens;
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
	while (before_count > 0 && isdigit(*ptr--)) {
		before_count--;
	}

	// Only add number if not escaped
	if (*(ptr - 1) != '\\') {
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
