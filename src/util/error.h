/**
 * @file util/error.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief Error handling defines and functions.
 */
#pragma once

#include "helper.h"

// Built-in and meta exit codes
typedef enum {
	CMDRES_OK = 0,
	CMDRES_GENERAL = 1,
	CMDRES_USAGE = 2,
	CMDRES_EXIT = 128,
} cmd_res;

/**
 * @brief Print shell fatal message and exit.
 *
 * @param[in] format - Printf format string.
 * @param[in] ... - Printf variable args.
 */
noreturn void print_fatal_hcf(const char *format, ...);

/**
 * @brief Print shell error.
 *
 * @param[in] format - Printf format string.
 * @param[in] ... - Printf variable args.
 */
void print_error(const char *format, ...);

/**
 * @brief Print shell warning.
 *
 * @param[in] format - Printf format string.
 * @param[in] ... - Printf variable args.
 */
void print_warning(const char *format, ...);

/**
 * @brief Store mesh's invokation name.
 *
 * @param[in] name - argv[0]
 */
void set_argv0(const char *const *argv0);
