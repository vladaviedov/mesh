/**
 * @file core/eval.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief AST evaluation and argv parsing.
 */
#pragma once

#include "../grammar/ast.h"

/**
 * @brief Evaluate an AST.
 *
 * @param[in] root - AST root.
 * @return Evaluation result.
 */
int eval_ast(ast_node *root);
