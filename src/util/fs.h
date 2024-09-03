/**
 * @file util/fs.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief Filesystem utility functions.
 */
#pragma once

/**
 * @brief Concatinate file paths.
 *
 * @param[in,out] path1 - First part of path and output buffer.
 * @param[in] path2 - Second part of path.
 * @note 'path1' must have PATH_MAX bytes allocated.
 */
void fs_path_cat(char *path1, const char *path2);

/**
 * @brief Allocate enough space for a file path.
 *
 * @return Buffer.
 * @note Allocated value.
 */
char *fs_path_malloc(void);

/**
 * @brief Equilavent of the 'mkdir -p' shell command.
 *
 * @param[in] path - Directory path.
 * @return 0 on success; -1 on error.
 */
int fs_mkdir_p(const char *path);

/**
 * @brief Get absolute file location of the context store directory.
 *
 * @return File path to the directory.
 */
const char *fs_conf_ctx(void);
