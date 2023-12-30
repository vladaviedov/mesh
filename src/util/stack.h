#pragma once

#include <stdint.h>

#include "vector.h"

typedef vector stack;

/**
 * @brief Create new stack.
 *
 * @param[in] type_size - Size of stored type.
 * @return New stack.
 * @note Allocated return value.
 */
stack *stack_new(uint64_t type_size);

/**
 * @brief Delete stack.
 *
 * @param[in/out] st - Stack object.
 */
void stack_free(stack *st);

/**
 * @brief Delete stack but free all its elements first.
 *
 * @param[in/out] st - Stack object.
 */
void stack_free_with_elements(stack *st);

/**
 * @brief Push item to the stack.
 *
 * @param[in/out] st - Stack object.
 * @param[in] value - Value to store.
 */
void stack_push(stack *st, const void *value);

/**
 * @brief Pop item from the stack.
 *
 * @param[in/out] st - Stack object.
 * @param[out] element - Popped element, if not NULL>
 * @return 0 on success; -1 on failure.
 */
int stack_pop(stack *st, void *element);

/**
 * @brief Get topmost element without popping.
 *
 * @param[in] st - Stack object.
 * @return Top element; NULLL on error.
 */
void *stack_peek(const stack *st);
