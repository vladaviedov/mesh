#include "vector.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"

#define EXP_BASE 2

#define ptr_at(vec, index) (vec->raw + vec->_type_size * index)

vector *vec_new(uint64_t type_size) {
	return vec_new_with_size(type_size, 0);
}

vector *vec_new_with_size(uint64_t type_size, uint32_t vec_size) {
	vector *vec = malloc(sizeof(vector));

	vec->raw = vec_size ? malloc(type_size * vec_size) : NULL;
	vec->count = 0;
	vec->_type_size = type_size;
	vec->_alloc_count = vec_size;

	return vec;
}

void vec_free(vector *vec) {
	if (vec == NULL) {
		return;
	}

	if (vec->raw != NULL) {
		free(vec->raw);
	}

	free(vec);
}

void vec_free_with_elements(vector *vec) {
	if (vec == NULL) {
		return;
	}

	// Free all elements
	for (uint32_t i = 0; i < vec->count; i++) {
		free(vec_at_deref(vec, i));
	}

	vec_free(vec);
}

void vec_push(vector *vec, const void *value) {
	if (vec == NULL) {
		return;
	}

	// Need to grow vector
	if (vec->count == vec->_alloc_count) {
		vec->_alloc_count = (vec->raw == NULL)
			? EXP_BASE
			: vec->_alloc_count * EXP_BASE;

		vec->raw = realloc(vec->raw, vec->_type_size * vec->_alloc_count);

		// Zero memory: ensures exec gets null-terminated array
		size_t added_len = (vec->_alloc_count - vec->count) * vec->_type_size;
		memset(ptr_at(vec, vec->count), 0, added_len);
	}

	// Copy element in
	void *push_ptr = ptr_at(vec, vec->count);
	memcpy(push_ptr, value, vec->_type_size);
	
	vec->count++;
}

int vec_pop(vector *vec, uint32_t index, void *element) {
	if (vec == NULL || index >= vec->count) {
		return -1;
	}

	// Copy element if buffer provided
	if (element != NULL) {
		memcpy(element, ptr_at(vec, index), vec->_type_size);
	}

	// Move all elements back
	for (uint32_t i = index + 1; i < vec->count; i++) {
		memcpy(ptr_at(vec, i - 1), ptr_at(vec, i), vec->_type_size);
	}

	vec->count--;
	return 0;
}

void *vec_at(const vector *vec, uint32_t index) {
	if (vec == NULL || index >= vec->count) {
		return NULL;
	}

	return ptr_at(vec, index);
}

void *vec_at_deref(const vector *vec, uint32_t index) {
	void *ptr = vec_at(vec, index);
	return ptr == NULL
		? NULL
		: *(void **)ptr;
}
