#include "lexer.h"

#include <ctype.h>
#include <stdint.h>
#include <stddef.h>

#include "../util/parsing.h"
#include "../util/vector.h"

token parse_input_redir(const char *hit, uint32_t before_count);
token parse_output_redir(const char *hit, uint32_t before_count);

token_vec *lexer_run(const char *input) {
	token_vec *tokens = vec_new(sizeof(token_vec));

	char ch;
	quote_st quotes = QUOTE_NONE;
	uint32_t length = 0;
	/* const char *token_start = input; */
	
	while ((ch = *input) != '\0') {
		// Parse redirections
		if (ch == '<' || ch == '>') {
			token redir = (ch == '<')
				? parse_input_redir(input, length)
				: parse_output_redir(input, length);

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
			default:
				break;
		}

		length++;
		input++;
	}

	return tokens;
}

token parse_input_redir(const char *hit, uint32_t before_count) {
	// TODO: implement
	token t = {
		.type = REDIR,
		.start = NULL,
		.length = 0
	};

	return t;
}

token parse_output_redir(const char *hit, uint32_t before_count) {
	// TODO: implement
	token t = {
		.type = REDIR,
		.start = NULL,
		.length = 0
	};

	return t;
}
