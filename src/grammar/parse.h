/**
 * @file grammar/parse.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief YACC parser interface.
 */
#pragma once

#include "ast.h"

/**
 * @brief Run the mesh parser on a string input.
 *
 * @param[in] str - Input string.
 * @return Generated AST.
 */
ast_node *parse_from_string(const char *str);
