#pragma once

#include <stddef.h>

/**
 * @brief Malloc requested space and attach a null-terminator to the end.
 *
 * @param[in] count - Count of items of size 'type_size'.
 * @param[in] type_size - Size of type.
 * @note Allocated return value.
 */
void *ntmalloc(size_t count, size_t type_size);

/**
 * @brief Print shell error.
 *
 * @param[in] format - Printf format string.
 * @param[in] ... - Printf variable args.
 */
void print_error(const char *format, ...);
