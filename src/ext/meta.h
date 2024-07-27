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

/**
 * @brief Run shell meta command.
 *
 * @param[in] args - Argument vector.
 * @param[out] command - Output command; NULL if not needed.
 * @return 0 on success; -1 on error.
 * @note When successful and command != NULL, command will be allocated.
 */
int run_meta(string_vector *args, char **command);
