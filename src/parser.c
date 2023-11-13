#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "vector.h"

#define TEMP_BUF_LEN 1024

// Track quote state in substitution
typedef enum {
	QUOTE_NONE,
	QUOTE_SINGLE,
	QUOTE_DOUBLE
} quote_st;

// Buffer used for parsing
static char temp[TEMP_BUF_LEN];
#define clear_temp() memset(temp, 0, TEMP_BUF_LEN)
#define add_temp(ch, ctr) { \
	temp[ctr] = ch; \
	ctr++; \
}
#define is_blank(x) (x == ' ' || x == '\t' || x == '\0')

str_vec *parser_split(char *input_string) {
	str_vec *vec = vec_new(sizeof(char *));

	// Setup
	clear_temp();
	uint32_t temp_index = 0;
	int quote = 0;
	int empty = 0;
	char *ptr_buf = NULL;

	char ch;
	while ((ch = *input_string++) != '\0') {
		switch (ch)	 {
			// Separating characters
			case ' ': // fallthrough
			case '\t':
				if (!quote && !empty) {
					// Insert string or empty string
					ptr_buf = temp_index == 0
						? strdup("")
						: strdup(temp);
					vec_push(vec, &ptr_buf);
					// Reset temp
					clear_temp();
					temp_index = 0;
					empty = 1;
				} else if (!empty) {
					add_temp(ch, temp_index);
				}
				break;
			// Quotes - only care about double
			case '\"':
				quote = !quote;
				empty = 0;
				break;
			// Escaped & Normal chars
			case '\\':
				ch = *input_string++;
				// fallthrough
			default:
				empty = 0;
				add_temp(ch, temp_index);
				break;
		}
	}

	if (!empty) {
		if (quote) {
			return NULL;
			// TODO: report error
		}

		ptr_buf = temp_index == 0
			? strdup("")
			: strdup(temp);
		vec_push(vec, &ptr_buf);
	}

	return vec;
}

char *parser_sub(char *input_string) {
	if (input_string == NULL) {
		return NULL;
	}

	clear_temp();
	uint32_t temp_index = 0;
	int noexpand = 0;
	int comment = 0;
	quote_st quotes = QUOTE_NONE;

	char ch;
	char prev = '\0';
	while (!comment && (ch = *input_string++) != '\0') {
		switch (ch) {
			// Don't touch escaped chars for now
			case '\\':
				add_temp(ch, temp_index)
				add_temp(*input_string++, temp_index);
				break;
			case '$':
				// TODO: implement
				break;
			case '\'':
				// Convert single quotes to double quotes unless enclosed
				switch (quotes) {
					case QUOTE_NONE:
						noexpand = 1;
						quotes = QUOTE_SINGLE;
						add_temp('\"', temp_index);
						break;
					case QUOTE_SINGLE:
						noexpand = 0;
						quotes = QUOTE_NONE;
						add_temp('\"', temp_index);
						break;
					case QUOTE_DOUBLE:
						add_temp('\'', temp_index);
						break;
				}
				break;
			case '\"':
				switch (quotes) {
					case QUOTE_NONE:
						quotes = QUOTE_DOUBLE;
						add_temp('\"', temp_index);
						break;
					case QUOTE_DOUBLE:
						quotes = QUOTE_NONE;
						add_temp('\"', temp_index);
						break;
					case QUOTE_SINGLE:
						// Escape quote for splitter
						add_temp('\\', temp_index);
						add_temp('\"', temp_index);
						break;
				}
				break;
			case '#':
				if (!quotes && is_blank(prev)) {
					comment = 1;
					break;
				}
				// fallthrough
			default:
				add_temp(ch, temp_index);
				break;

		}
	}

	return strdup(temp);
}
