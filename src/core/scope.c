#include "scope.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <c-utils/vector.h>
#include <c-utils/stack.h>

#include "../util/helper.h"

typedef union {
	char *name;
	uint32_t pos;
} id;

typedef struct {
	id key;
	char *value;
	int is_pos;
} scoped_var;

typedef vector scoped_var_vector;

typedef struct {
	scoped_var_vector vars;
	uint32_t pos_count;
} scope;

typedef stack scope_stack;

// Stack of scopes
static scope_stack *scopes = NULL;
// Current scope
static scope frame;

void init_frame(void);
scoped_var *find_named_var(const char *key, uint32_t *index);
scoped_var *find_pos_var(uint32_t key, uint32_t *index);

void scope_init(void) {
	scopes = stack_new(sizeof(scope));
	init_frame();
}

void scope_set_var(const char *key, const char *value) {
	scoped_var *find_res = find_named_var(key, NULL);

	if (find_res == NULL) {
		// New variable
		scoped_var new_var = {
			.key.name = strdup(key),
			.value = (value == NULL) ? strdup("") : strdup(value),
			.is_pos = 0,
		};

		vec_push(&frame.vars, &new_var);
	} else {
		// Update existing
		free(find_res->value);
		find_res->value = strdup(value);
	}
}

const char *scope_get_var(const char *key) {
	scoped_var *find_res = find_named_var(key, NULL);
	if (find_res == NULL) {
		return NULL;
	}

	return find_res->value;
}

int scope_delete_var(const char *key) {
	uint32_t index;
	scoped_var *find_res = find_named_var(key, &index);
	if (find_res == NULL) {
		return -1;
	}

	return vec_erase(&frame.vars, index, NULL);
}

uint32_t scope_pos_count(void) {
	return frame.pos_count;
}

void scope_append_pos(const char *value) {
	scoped_var new_pos = {
		// 1-indexed
		.key.pos = frame.pos_count + 1,
		.value = strdup(value),
		.is_pos = 1
	};

	vec_push(&frame.vars, &new_pos);
	frame.pos_count++;

	// Update variables
	char *list = scope_list_pos();
	scope_set_var("@", list);
	free(list);

	char buffer[32];
	snprintf(buffer, 32, "%d", frame.pos_count);
	scope_set_var("#", buffer);
}

const char *scope_get_pos(uint32_t index) {
	scoped_var *find_res = find_pos_var(index, NULL);
	if (find_res == NULL) {
		return NULL;
	}

	return find_res->value;
}

char *scope_list_pos(void) {
	if (frame.pos_count == 0) {
		return NULL;
	}

	uint32_t total_length = 0;
	const char *pos_vars[frame.pos_count];
	for (uint32_t i = 0; i < frame.pos_count; i++) {
		// 1-indexed vars
		scoped_var *pos_var = find_pos_var(i + 1, NULL);
		pos_vars[i] = pos_var->value;
		total_length += strlen(pos_var->value) + 1;
	}

	char *list = calloc(total_length, sizeof(char));

	// Put first one in without a space
	strcpy(list, pos_vars[0]);
	for (uint32_t i = 1; i < frame.pos_count; i++) {
		strcat(list, " ");
		strcat(list, pos_vars[i]);
	}

	return list;
}

void scope_reset_pos(void) {
	uint32_t vec_i;
	for (uint32_t i = 0; i < frame.pos_count; i++) {
		find_pos_var(i, &vec_i);
		vec_erase(&frame.vars, vec_i, NULL);
	}

	frame.pos_count = 0;
}

void scope_create_frame(void) {
	// Save scope
	stack_push(scopes, &frame);

	// Create new frame
	init_frame();
}

int scope_delete_frame(void) {
	if (scopes->count == 0) {
		print_error("cannot delete top-level scope");
		return -1;
	}

	// Delete scope
	vec_deinit(&frame.vars);

	// Load old scope
	stack_pop(scopes, &frame);

	return 0;
}

/** Internal */

/**
 * @brief Initialize a new scope frame.
 */
void init_frame(void) {
	frame.vars = vec_init(sizeof(scoped_var));
	frame.pos_count = 0;

	scope_set_var("#", "0");
}

/**
 * @brief Find variable by name.
 *
 * @param[in] key - Variable name.
 * @param[out] index - If not NULL and item was found, places index here.
 * @return Scoped nonpositional variable; NULL on error.
 */
scoped_var *find_named_var(const char *key, uint32_t *index) {
	for (uint32_t i = 0; i < frame.vars.count; i++) {
		scoped_var *entry = vec_at_mut(&frame.vars, i);
		if (entry->is_pos) {
			continue;
		}

		if (strcmp(entry->key.name, key) == 0) {
			if (index != NULL) {
				*index = i;
			}
			return entry;
		}
	}

	return NULL;
}

/**
 * @brief Find variable by position index.
 *
 * @param[in] key - Position index.
 * @param[in] index - If not NULL and item was found, places index here.
 * @return Scoped positional variable; NULL on error.
 */
scoped_var *find_pos_var(uint32_t key, uint32_t *index) {
	for (uint32_t i = 0; i < frame.vars.count; i++) {
		scoped_var *entry = vec_at_mut(&frame.vars, i);
		if (!entry->is_pos) {
			continue;
		}

		if (entry->key.pos == key) {
			if (index != NULL) {
				*index = i;
			}
			return entry;
		}
	}

	return NULL;
}
