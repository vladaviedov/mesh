#pragma once

#include "../util/vector.h"

/**
 * @brief Determines how to run a command and runs it.
 *
 * @param[in] args - Processed argument vector.
 * @return Status code.
 */
int run_dispatch(str_vec *args);
