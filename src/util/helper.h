#pragma once

#include <stddef.h>

#include <c-utils/vector.h>

#ifdef __GNUC__
#define unused __attribute__((unused))
#else
#define unused
#endif

typedef vector string_vector;

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

/**
 * @brief Print shell warning.
 *
 * @param[in] format - Printf format string.
 * @param[in] ... - Printf variable args.
 */
void print_warning(const char *format, ...);

/**
 * @brief Free vector and its elements.
 *
 * @param[in] vec - Vector object.
 */
void free_with_elements(vector *vec);
