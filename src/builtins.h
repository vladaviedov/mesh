#pragma once

#include "vector.h"

/**
 * @brief Assign requested variables.
 *
 * @param[in] args - Argument vector.
 * @param[in] export_flag - If variables should be exported.
 * @return 0 on success; -1 on failure.
 */
int pure_assign(str_vec *args, int export_flag);

/**
 * @brief Run shell builtin.
 *
 * @param[in] args - Argument vector.
 * @return Exit code.
 */
int run_builtin(str_vec *args);
