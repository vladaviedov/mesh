#include "context.h"

#include <stdlib.h>
#include <string.h>

#include "../util/helper.h"
#include "../util/vector.h"

typedef vector context_vector;
static context_vector *contexts;

static context *current_ctx = NULL;

context *find_context(const char *name);

int context_new(const char *name, context **ctx_out) {
	if (contexts == NULL) {
		contexts = vec_new(sizeof(context));
	}
	if (find_context(name) != NULL) {
		return -1;
	}

	context *ctx = malloc(sizeof(context));

	ctx->name = name;
	ctx->commands = vec_new(sizeof(char *));

	vec_push(contexts, ctx);
	if (ctx_out != NULL) {
		*ctx_out = ctx;
	}

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

const context *context_get(void) {
	return current_ctx;
}

const char *context_get_row_abs(uint32_t index) {
	if (current_ctx == NULL) {
		print_error("context is not set");
		return NULL;
	}

	return fix_ptr(vec_at(current_ctx->commands, index));
}

const char *context_get_row_rel(uint32_t index) {
	if (current_ctx == NULL) {
		print_error("context is not set");
		return NULL;
	}

	return fix_ptr(vec_at(current_ctx->commands, current_ctx->commands->count - index - 1));
}

int context_add(const char *command, context *ctx) {
	if (ctx == NULL) {
		if (current_ctx == NULL) {
			print_error("context is not set");
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
 * @return Context object; NULL on error.
 */
context *find_context(const char *name) {
	for (uint32_t i = 0; i < contexts->count; i++) {
		context *ctx = vec_at(contexts, i);
		if (strcmp(ctx->name, name) == 0) {
			return ctx;
		}
	}

	return NULL;
}
