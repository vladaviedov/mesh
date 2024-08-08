/**
 * @file ext/meta.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Mesh meta-commands.
 */
#pragma once

#include <c-utils/vector.h>

#include "../util/helper.h"

typedef struct meta meta;

/**
 * @brief Search the meta registry by name.
 *
 * @param[in] name - Name of command.
 * @return Meta command identifier; NULL if not found.
 */
const meta *search_meta(const char *name);

/**
 * @brief Run shell meta command.
 *
 * @param[in] cmd - Meta command identifier.
 * @param[in] args - Argument vector.
 * @param[out] command - Output command; NULL if not needed.
 * @return 0 on success; -1 on error.
 * @note When successful and command != NULL, command will be allocated.
 */
int run_meta(const meta *cmd, string_vector *args, char **command);
