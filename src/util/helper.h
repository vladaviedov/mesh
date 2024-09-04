/**
 * @file util/helper.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Utility helper functions.
 */
#pragma once

#include <c-utils/vector.h>

#ifdef __GNUC__
#define unused __attribute__((unused))
#define noreturn __attribute__((noreturn))
#else
#define unused
#define noreturn
#endif

typedef vector string_vector;
typedef vector char_vector;

extern char null_char;

/**
 * @brief Malloc requested space and attach a null-terminator to the end.
 *
 * @param[in] count - Count of items of size 'type_size'.
 * @param[in] type_size - Size of type.
 * @note Allocated return value.
 */
void *ntmalloc(size_t count, size_t type_size);

/**
 * @brief Free vector's pointer elements.
 *
 * @param[in] vec - Vector object.
 */
void free_elements(vector *vec);
