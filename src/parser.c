#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "vector.h"

#define TEMP_BUF_LEN 1024

typedef enum {
	QUOTE_NONE,
	QUOTE_SINGLE,
	QUOTE_DOUBLE
} quote_st;

// Buffer used for parsing
static char temp[TEMP_BUF_LEN];
#define clear_temp() memset(temp, 0, TEMP_BUF_LEN); temp_index = 0

str_vec *parser_split(char *input_string) {
	str_vec *vec = vec_new(sizeof(char *));

	// Setup
	uint32_t temp_index = 0;
	clear_temp();
	quote_st quote = QUOTE_NONE;
	int empty = 0;
	char *ptr_buf = NULL;

	char ch;
	while ((ch = *input_string++) != '\0') {
		switch (ch)	 {
			// Separating characters
			case ' ': // fallthrough
			case '\t':
				if (quote == QUOTE_NONE && !empty) {
					// Insert string or empty string
					ptr_buf = temp_index == 0
						? strdup("")
						: strdup(temp);
					vec_push(vec, &ptr_buf);
					clear_temp();
					empty = 1;
				} else if (!empty) {
					temp[temp_index] = ch;
					temp_index++;
				}
				break;
			// Quotes
			case '\'':
				switch (quote) {
					case QUOTE_NONE:
						empty = 0;
						quote = QUOTE_SINGLE;
						break;
					case QUOTE_SINGLE:
						quote = QUOTE_NONE;
						break;
					case QUOTE_DOUBLE:
						temp[temp_index] = ch;
						temp_index++;
						break;
				}
				break;
			case '\"':
				switch (quote) {
					case QUOTE_NONE:
						empty = 0;
						quote = QUOTE_DOUBLE;
						break;
					case QUOTE_DOUBLE:
						quote = QUOTE_NONE;
						break;
					case QUOTE_SINGLE:
						temp[temp_index] = ch;
						temp_index++;
						break;
				}
				break;
			// Escaped & normal characters
			case '\\':
				ch = *input_string++;
				// fallthrough
			default:
				empty = 0;
				temp[temp_index] = ch;
				temp_index++;
				break;
		}
	}

	if (!empty) {
		if (quote != QUOTE_NONE) {
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
