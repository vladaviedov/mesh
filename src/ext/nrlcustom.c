/**
 * @file ext/nrlcustom.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version ?
 * @date 2025
 * @license GPLv3.0
 * @brief Custom handlers for nanorl.
 */
#define _POSIX_C_SOURCE 200809L
#include "nrlcustom.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <nanorl/escape.h>
#include <nanorl/nanorl.h>

#include "context.h"

static nrl_state history_arrow(const nrl_state *state, nrl_escape code);

static nrl_escape_handler arrow_up = {
	.id = NRL_ESC_KEY_UP,
	.format = NRL_DF_UTF8,
	.func = &history_arrow,
};
static nrl_escape_handler arrow_down = {
	.id = NRL_ESC_KEY_DOWN,
	.format = NRL_DF_UTF8,
	.func = &history_arrow,
};
static nrl_escape_handler *handlers[] = { &arrow_up, &arrow_down, NULL };

static uint32_t arrow_pos = 0;

void nrlcustom_register(nrl_config *config) {
	config->custom_handlers = handlers;
	config->custom_ignore_default = true;
}

void nrlcustom_reset(void) {
	arrow_pos = 0;
}

static nrl_state history_arrow(const nrl_state *state, nrl_escape code) {
	const context *ctx = context_get("history");

	if (code == NRL_ESC_KEY_UP && arrow_pos < ctx->commands.count) {
		arrow_pos++;
	} else if (code == NRL_ESC_KEY_DOWN && arrow_pos > 0) {
		arrow_pos--;
	} else {
		nrl_state same = {
			.line.utf8_line = strdup(state->line.utf8_line),
			.cursor = state->cursor,
		};

		return same;
	}

	if (arrow_pos == 0) {
		nrl_state new_state = {
			.line.utf8_line = strdup(""),
			.cursor = 0,
		};

		return new_state;
	}

	char *const *cmd = vec_at(&ctx->commands, ctx->commands.count - arrow_pos);
	nrl_state new_state = {
		.line.utf8_line = strdup(*cmd),
		.cursor = strlen(*cmd),
	};

	return new_state;
}
