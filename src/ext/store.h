/**
 * @file ext/store.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief Mesh context store access.
 */
#pragma once

#include <c-utils/vector.h>

typedef struct {
	char *filename;
	char *name;
} store_item;

typedef vector store_item_vector;

/**
 * @brief Load store from disk.
 *
 * @param[in] path - Base filepath for stored contexts.
 * @return 0 on success; -1 on failure.
 */
int store_load(const char *path);

/**
 * @brief Get store item by name.
 *
 * @param[in] name - Context name.
 * @return Store item.
 */
const store_item *store_get(const char *name);

/**
 * @brief List all loaded contexts.
 *
 * @return Store item vector.
 */
const store_item_vector *store_get_list(void);
