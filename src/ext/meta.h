#pragma once

#include "../util/vector.h"

/**
 * @brief Run shell meta command.
 *
 * @param[in] args - Argument vector.
 * @param[out] command - Output command; NULL if not needed.
 * @return 0 on success; -1 on error.
 * @note When successful and command != NULL, command will be allocated.
 */
int run_meta(str_vec *args, char **command);
