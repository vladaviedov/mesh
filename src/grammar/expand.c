#include "expand.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <c-utils/vector.h>
#include <c-utils/vector-ext.h>

#include "../util/helper.h"
#include "../core/vars.h"
#include "../core/scope.h"
#include "../core/exec.h"

#define RD_BUF_LEN 1024

static char *parse_command(const char *start, const char **end);
static char *parse_variable(const char *start, const char **end);
static char *subshell_eval(const char *command);

char *expand_word(const char *word) {
	char_vector expanded = vec_init(sizeof(char));

	int noexpand = 0;
	uint32_t pending = 0;

	const char *trav = word;
	char ch;
	while ((ch = *trav++) != '\0') {
		switch (ch) {
		case '\\':
			// Don't touch escaped characters yet
			pending += 2;
			break;
		case '\'':
			noexpand = !noexpand;
			pending++;
			break;
		case '$':
			if (noexpand) {
				pending++;
				break;
			}

			// Flush pending
			vec_bulk_push(&expanded, word, pending);
			word += pending;
			pending = 0;

			ch = *trav;
			if (ch == '(') {
				char *command = parse_command(++trav, &word);
				char *result = subshell_eval(command);
				vec_bulk_push(&expanded, result, strlen(result));
				free(command);
				free(result);
			} else if (isdigit(ch)) {
				uint32_t position = strtoul(trav, (char **)&word, 10);
				const char *value = scope_get_pos(position);
				if (value != NULL) {
					vec_bulk_push(&expanded, value, strlen(value));
				}
			} else {
				char *var_name = parse_variable(trav, &word);

				// Try scope first
				const char *value = scope_get_var(var_name);
				if (value == NULL) {
					// Then check the environment
					value = vars_get(var_name);
				}

				if (value != NULL) {
					vec_bulk_push(&expanded, value, strlen(value));
				}
				free(var_name);
			}
			break;
		default:
			pending++;
			break;
		}
	}

	if (pending > 0) {
		vec_bulk_push(&expanded, word, pending);
	}

	// Null-terminate string
	vec_push(&expanded, &null_char);
	return vec_collect(&expanded);
}

static char *parse_command(const char *start, const char **end) {
	char *closing = strchr(start, ')');
	// TODO: does this happen?
	if (closing == NULL) {
		return NULL;
	}

	*end = closing + 1;
	return strndup(start, closing - start);
}

static char *parse_variable(const char *start, const char **end) {
	// Handle special variables
	switch (*start) {
	case '$':
	case '?':
	case '#':
	case '@':
		*end = start + 1;
		return strndup(start, 1);
	default:
		break;
	}

	// Traverse string until a non alphanumeric char
	const char *trav = start;
	while (isalnum(*trav)) {
		trav++;
	}

	*end = trav;
	return strndup(start, trav - start);
}

static char *subshell_eval(const char *command) {
	int pipe_fds[2];
	if (pipe(pipe_fds) < 0) {
		return NULL;
	}

	int flags = fcntl(pipe_fds[0], F_GETFL, 0);
	fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK);

	exec_subshell(command, pipe_fds[1]);

	char_vector output = vec_init(sizeof(char));
	char rd_buf[RD_BUF_LEN];
	ssize_t size;
	while ((size = read(pipe_fds[0], rd_buf, RD_BUF_LEN)) > 0) {
		vec_bulk_push(&output, rd_buf, size);
	}

	close(pipe_fds[0]);
	close(pipe_fds[1]);

	if (output.count == 0) {
		vec_deinit(&output);
		return NULL;
	}
	
	vec_push(&output, &null_char);
	char *data = vec_collect(&output);
	return data;
}