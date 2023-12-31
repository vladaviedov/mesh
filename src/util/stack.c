#include "stack.h"

#include <stdint.h>

#include "vector.h"

stack *stack_new(uint64_t type_size) {
	return vec_new(type_size);
}

void stack_free(stack *st) {
	return vec_free(st);
}

void stack_free_with_elements(stack *st) {
	return vec_free_with_elements(st);
}

void stack_push(stack *st, const void *value) {
	return vec_push(st, value);
}

int stack_pop(stack *st, void *element) {
	return vec_pop(st, st->count - 1, element);
}

void *stack_peek(const stack *st) {
	return vec_at(st, st->count - 1);
}
