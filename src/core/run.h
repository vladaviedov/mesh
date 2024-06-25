#pragma once

#include <c-utils/vector.h>

#include "../util/helper.h"

/**
 * @brief Determines how to run a command and runs it.
 *
 * @param[in] args - Processed argument vector.
 * @return Status code.
 */
int run_dispatch(string_vector *args);
