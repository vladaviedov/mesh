/**
 * @file util/helper.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2023-2024
 * @license GPLv3.0
 * @brief Utility helper functions.
 */
#pragma once

#include <stddef.h>

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
 * @brief Store mesh's invokation name.
 *
 * @param[in] name - argv[0]
 */
void set_argv0(const char *const *argv0);

/**
 * @brief Print shell fatal message and exit.
 *
 * @param[in] format - Printf format string.
 * @param[in] ... - Printf variable args.
 */
noreturn void print_fatal_hcf(const char *format, ...);

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
 * @brief Free vector's pointer elements.
 *
 * @param[in] vec - Vector object.
 */
void free_elements(vector *vec);

/**
 * @brief Get mesh config path.
 *
 * @return Config absolute filepath.
 * @return Allocated return value.
 */
char *config_path(void);

/**
 * @brief Combine two parts together.
 *
 * @param[in] path1 - First part.
 * @param[in] path2 - Second part.
 * @return Allocated return value.
 */
char *path_combine(const char *path1, const char *path2);
