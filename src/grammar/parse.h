#pragma once

#include "ast.h"

/**
 * @brief Run the mesh parser on a string input.
 *
 * @param[in] str - Input string.
 * @return Generated AST.
 */
ast_node *parse_from_string(const char *str);
