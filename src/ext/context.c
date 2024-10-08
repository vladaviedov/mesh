/**
 * @file ext/context.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Mesh context manipulation.
 */
#define _POSIX_C_SOURCE 200809L
#include "context.h"

#include <stdlib.h>
#include <string.h>

#include <c-utils/vector.h>

#include "../util/error.h"
#include "../util/helper.h"

static context_vector *contexts = NULL;
static context *current_ctx = NULL;
static context *history = NULL;

static int find_context_idx(const char *name, uint32_t *index);
static context *find_context(const char *name);

int context_new(const char *name, context **ctx_out) {
	if (contexts == NULL) {
		contexts = vec_new(sizeof(context));
	}
	if (find_context(name) != NULL) {
		return -1;
	}

	char *current_name = (current_ctx != NULL) ? current_ctx->name : NULL;

	context ctx = {
		.name = strdup(name),
		.commands = vec_init(sizeof(char *)),
	};

	vec_push(contexts, &ctx);
	if (ctx_out != NULL) {
		*ctx_out = vec_at_mut(contexts, contexts->count - 1);
	}

	// Vector might realloc the memory
	if (current_name != NULL) {
		current_ctx = find_context(current_name);
	}
	if (history != NULL) {
		history = find_context("history");
	}

	return 0;
}

int context_delete(const char *name) {
	uint32_t index;
	if (!find_context_idx(name, &index)) {
		return -1;
	}

	// Reset current context if deleting it
	char *current_name = current_ctx->name;
	if (vec_at(contexts, index) == current_ctx) {
		current_name = NULL;
		current_ctx = NULL;
	}

	// Remove from element
	context deleted;
	if (vec_erase(contexts, index, &deleted) < 0) {
		return -1;
	}

	// Delete context
	free(deleted.name);
	free_elements(&deleted.commands);
	vec_deinit(&deleted.commands);

	// Re-find contexts
	if (current_name != NULL) {
		current_ctx = find_context(current_name);
	}
	history = find_context("history");

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

const char *context_get_row(int32_t index) {
	if (current_ctx == NULL) {
		print_error("context is not set\n");
		return NULL;
	}

	int32_t abs_index
		= (index < 0) ? (int32_t)current_ctx->commands.count + index : index;
	if (abs_index < 0) {
		return NULL;
	}

	char *const *item = vec_at(&current_ctx->commands, (uint32_t)abs_index);
	return (item != NULL) ? *item : NULL;
}

int context_add(const char *command, context *ctx) {
	if (ctx == NULL) {
		if (current_ctx == NULL) {
			print_error("context is not set\n");
			return -1;
		}

		ctx = current_ctx;
	}

	vec_push(&ctx->commands, &command);
	return 0;
}

int context_replace(const char *new_cmd, uint32_t index, context *ctx) {
	if (ctx == NULL) {
		if (current_ctx == NULL) {
			print_error("context is not set\n");
			return -1;
		}

		ctx = current_ctx;
	}

	if (index >= ctx->commands.count) {
		print_error("index is out of bounds\n");
		return -1;
	}

	char **old_cmd = vec_at_mut(&ctx->commands, index);
	free(*old_cmd);

	// Recast for const-correctness
	const char **cmd_slot = (const char **)old_cmd;
	*cmd_slot = new_cmd;

	return 0;
}

int context_hist_init(void) {
	return context_new("history", &history);
}

int context_hist_add(const char *command) {
	return context_add(command, history);
}

/** Internal functions */

/**
 * @brief Find context by name.
 *
 * @param[in] name - Query.
 * @param[out] index - Index of element.
 * @return Boolean result - found or not.
 */
static int find_context_idx(const char *name, uint32_t *index) {
	for (uint32_t i = 0; i < contexts->count; i++) {
		const context *ctx = vec_at(contexts, i);
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
static context *find_context(const char *name) {
	uint32_t index;
	if (find_context_idx(name, &index)) {
		return vec_at_mut(contexts, index);
	}

	return NULL;
}
