#pragma once

#include <stdint.h>
#include <c-utils/vector.h>

#include "../util/helper.h"

/**
 * @brief Assign requested variables.
 *
 * @param[in] count - Variable count.
 * @param[in] args - Variable array.
 * @param[in] export_flag - If variables should be exported.
 * @return 0 on success; -1 on failure.
 */
int pure_assign(uint32_t count, char **args, int export_flag);

/**
 * @brief Run shell builtin.
 *
 * @param[in] args - Argument vector.
 * @return Exit code; -1 if not found.
 */
int run_builtin(string_vector *args);
