/**
 * @file core/run.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Figure out how to run a command.
 */
#pragma once

#include <c-utils/vector.h>

#include "../util/helper.h"
#include "flags.h"

/**
 * @brief Determines how to run a command and runs it.
 *
 * @param[in] args - Processed argument vector.
 * @param[in] flags - Flags for this execution.
 * @return Status code.
 */
int run_dispatch(string_vector *args, run_flags *flags);
