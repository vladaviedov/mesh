#include "parser.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../util/helper.h"
#include "../util/vector.h"
#include "vars.h"

#define TEMP_BUF_LEN 1024

// Track quote state in substitution
typedef enum {
	QUOTE_NONE,
	QUOTE_SINGLE,
	QUOTE_DOUBLE
} quote_st;

// Buffer used for parsing
static char temp[TEMP_BUF_LEN];
static uint32_t temp_index = 0;

void clear_temp(void);
void add_temp(char ch);
void insert_temp(const char *str);
uint32_t parse_var_name(const char *start, char *buffer);
uint32_t parse_pipe_cmd(const char *start, char *buffer);

#define is_blank(x) (x == ' ' || x == '\t' || x == '\0')

char *parser_sub(char *input_string) {
	if (input_string == NULL) {
		return NULL;
	}

	clear_temp();
	int noexpand = 0;
	int comment = 0;
	quote_st quotes = QUOTE_NONE;

	char ch;
	char prev = '\0';
	while (!comment && (ch = *input_string++) != '\0') {
		switch (ch) {
			// Don't touch escaped chars for now
			case '\\':
				add_temp(ch);
				add_temp(*input_string++);
				break;
			case '$':
				if (noexpand) {
					add_temp(ch);
				}

				char buffer[TEMP_BUF_LEN];
				ch = *input_string;
				if (ch == '(') {
					input_string += parse_pipe_cmd(input_string, buffer);
					// TODO: get value
					exit(0);
				} else {
					input_string += parse_var_name(input_string, buffer);
					const char *value = vars_get(buffer);
					if (value == NULL) {
						continue;
					}

					insert_temp(value);
				}

				break;
			case '\'':
				// Convert single quotes to double quotes unless enclosed
				switch (quotes) {
					case QUOTE_NONE:
						noexpand = 1;
						quotes = QUOTE_SINGLE;
						add_temp('\"');
						break;
					case QUOTE_SINGLE:
						noexpand = 0;
						quotes = QUOTE_NONE;
						add_temp('\"');
						break;
					case QUOTE_DOUBLE:
						add_temp('\'');
						break;
				}
				break;
			case '\"':
				switch (quotes) {
					case QUOTE_NONE:
						quotes = QUOTE_DOUBLE;
						add_temp('\"');
						break;
					case QUOTE_DOUBLE:
						quotes = QUOTE_NONE;
						add_temp('\"');
						break;
					case QUOTE_SINGLE:
						// Escape quote for splitter
						add_temp('\\');
						add_temp('\"');
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
				add_temp(ch);
				break;

		}
	}

	return strdup(temp);
}

str_vec *parser_split(char *input_string, char **end) {
	str_vec *vec = vec_new(sizeof(char *));

	// Setup
	clear_temp();
	int quote = 0;
	int empty = 1;
	int terminated = 0;
	char *ptr_buf = NULL;

	char ch;
	while (!terminated && (ch = *input_string++) != '\0') {
		switch (ch) {
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
					add_temp(ch);
				}
				break;
			// Colon is a command terminator
			case ';':
				if (!quote) {
					*end = input_string;
					terminated = 1;
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
				add_temp(ch);
				break;
		}
	}

	if (!empty) {
		if (quote) {
			print_error("parser: unterminated quotes");
			return NULL;
		}

		ptr_buf = temp_index == 0
			? strdup("")
			: strdup(temp);
		vec_push(vec, &ptr_buf);
	}

	// String fully parsed
	if (!terminated) {
		*end = NULL;
	}

	return vec;
}

int is_valid_var_name(const char *name) {
	// First chracter cannot be a number
	if (isdigit(*name)) {
		return 0;
	}

	// All characters must be alphanumeric
	char ch;
	while ((ch = *name++) != '\0') {
		if (!isalnum(ch)) {
			return 0;
		}
	}

	return 1;
}

int is_assign(const char *token) {
	char *equal_sign = strchr(token, '=');

	// Equal sign required
	if (equal_sign == NULL) {
		return 0;
	}

	// Check key
	size_t key_len = equal_sign - token;
	char key[key_len + 1];
	strncpy(key, token, key_len);
	key[key_len] = '\0';

	return is_valid_var_name(key);
}

int is_pure_assign(const str_vec *expression) {
	for (uint32_t i = 0; i < expression->count; i++) {
		if (!is_assign(fix_ptr(vec_at(expression, i)))) {
			return 0;
		}
	}

	return 1;
}

/** Temp buffer manipulation */

void clear_temp(void) {
	memset(temp, 0, TEMP_BUF_LEN);
	temp_index = 0;
}

void add_temp(char ch) {
	temp[temp_index] = ch;
	temp_index++;
}

void insert_temp(const char *str) {
	strcat(temp, str);
	temp_index += strlen(str);
}

uint32_t parse_var_name(const char *start, char *buffer) {
	uint32_t buffer_index = 0;

	while (1) {
		char ch = *start++;
		if (!isalnum(ch)) {
			// Handle special variables
			if (buffer_index == 0) {
				switch (ch) {
					case '$': // fallthrough
					case '?':
						buffer[buffer_index] = ch;
						buffer_index++;
						break;
					default:
						break;
				}
			}

			break;
		}

		buffer[buffer_index] = ch;
		buffer_index++;
	}

	buffer[buffer_index] = '\0';
	return buffer_index;
}

uint32_t parse_pipe_cmd(const char *start, char *buffer) {
	uint32_t buffer_index = 0;

	int end = 0;
	while (!end) {
		char ch = *start++;
		switch (ch) {
			case '\0': // fallthrough
			case ')':
				end = 1;
				break;
			default:
				buffer[buffer_index] = ch;
				buffer_index++;
				break;
		}
	}

	buffer[buffer_index] = '\0';
	return buffer_index;
}
