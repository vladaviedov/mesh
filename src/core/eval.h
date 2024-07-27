#pragma once

#include "../grammar/ast.h"

/**
 * @brief Evaluate an AST.
 *
 * @param[in] root - AST root.
 * @return Evaluation result.
 */
int eval_ast(ast_node *root);
