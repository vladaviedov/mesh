#include "context.h"

#include <stdlib.h>
#include <string.h>

#include "../util/helper.h"
#include "../util/vector.h"

static context_vector *contexts = NULL;
static context *current_ctx = NULL;

int find_context_idx(const char *name, uint32_t *index);
context *find_context(const char *name);

int context_new(const char *name, context **ctx_out) {
	if (contexts == NULL) {
		contexts = vec_new(sizeof(context *));
	}
	if (find_context(name) != NULL) {
		return -1;
	}

	context *ctx = malloc(sizeof(context));

	ctx->name = strdup(name);
	ctx->commands = vec_new(sizeof(char *));

	vec_push(contexts, &ctx);
	if (ctx_out != NULL) {
		*ctx_out = ctx;
	}

	return 0;
}

int context_delete(const char *name) {
	uint32_t index;
	if (!find_context_idx(name, &index)) {
		return -1;
	}

	// Reset current context if deleting it
	if (fix_ptr(vec_at(contexts, index)) == current_ctx) {
		current_ctx = NULL;
	}

	// Remove from element
	context *deleted;
	if (vec_pop(contexts, index, &deleted) < 0) {
		return -1;
	}

	// Delete context
	free((void *)deleted->name);
	vec_free_with_elements(deleted->commands);

	return 0;
}

int context_select(const char *name) {
	context *ctx = find_context(name);
	if (ctx == NULL) {
		return -1;
	}

	current_ctx = ctx;
	return 0;
}

const context *context_get(const char *name) {
	if (name == NULL) {
		if (current_ctx == NULL) {
			print_error("context is not set\n");
		}

		return current_ctx;
	}

	return find_context(name);
}

const context_vector *context_get_all(void) {
	return contexts;
}

const char *context_get_row_abs(uint32_t index) {
	if (current_ctx == NULL) {
		print_error("context is not set\n");
		return NULL;
	}

	return fix_ptr(vec_at(current_ctx->commands, index));
}

const char *context_get_row_rel(uint32_t index) {
	if (current_ctx == NULL) {
		print_error("context is not set\n");
		return NULL;
	}

	return fix_ptr(vec_at(current_ctx->commands, current_ctx->commands->count - index - 1));
}

int context_add(const char *command, context *ctx) {
	if (ctx == NULL) {
		if (current_ctx == NULL) {
			print_error("context is not set\n");
			return -1;
		}

		ctx = current_ctx;
	}

	vec_push(ctx->commands, &command);
	return 0;
}

/** Internal functions */

/**
 * @brief Find context by name.
 *
 * @param[in] name - Query.
 * @param[out] index - Index of element.
 * @return Boolean result - found or not.
 */
int find_context_idx(const char *name, uint32_t *index) {
	for (uint32_t i = 0; i < contexts->count; i++) {
		context *ctx = fix_ptr(vec_at(contexts, i));
		if (strcmp(ctx->name, name) == 0) {
			*index = i;
			return 1;
		}
	}

	return 0;
}

/**
 * @brief Find context by name.
 *
 * @param[in] name - Query.
 * @return Context object; NULL on error.
 */
context *find_context(const char *name) {
	uint32_t index;
	if (find_context_idx(name, &index)) {
		return fix_ptr(vec_at(contexts, index));
	}

	return NULL;
}
