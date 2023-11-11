#pragma once

#include <stdint.h>

typedef struct {
	void *raw;
	uint32_t count;

	uint64_t _type_size;
	uint32_t _alloc_count;
} vector;

/**
 * @brief Create new vector.
 *
 * @param[in] type_size - Size of stored type.
 * @return New vector.
 * @note Allocated return value.
 */
vector *vec_new(uint64_t type_size);

/**
 * @brief Create new vector with custom initial size.
 *
 * @param[in] type_size - Size of stored type.
 * @param[in] vec_size - Custom initial size.
 * @return New vector.
 * @note Allocated return value.
 */
vector *vec_new_with_size(uint64_t type_size, uint32_t vec_size);

/**
 * @brief Delete vector.
 *
 * @param[in/out] vec - Vector object.
 */
void vec_free(vector *vec);

/**
 * @brief Add new element to the end.
 *
 * @param[in/out] vec - Vector object.
 * @param[in] value - Value to add.
 */
void vec_push(vector *vec, const void *value);

/**
 * @brief Remove element at index.
 *
 * @param[in/out] vec - Vector object.
 * @param[in] index - Index.
 * @param[out] element - Popped element, if not NULL.
 * @return 0 on success; -1 on failure.
 */
int vec_pop(vector *vec, uint32_t index, void *element);

/**
 * @brief Get element at index.
 *
 * @param[in] vec - Vector object.
 * @param[in] index - Index.
 * @return Element at index; NULL on error.
 */
void *vec_at(const vector *vec, uint32_t index);
