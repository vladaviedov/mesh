#include "ast.h"

#include <stdlib.h>
#include <string.h>

static ast_node *ast_make_node(
		ast_kind kind,
		ast_value value,
		ast_node *left,
		ast_node *right);

static ast_node *ast_make_noval_node(
		ast_kind kind,
		ast_node *left,
		ast_node *right);

ast_node *ast_make_seq(ast_seq_value value, ast_node *left, ast_node *right) {
	ast_value astv = { .seq = value };
	return ast_make_node(AST_KIND_SEQ, astv, left, right);
}

ast_node *ast_make_cond(ast_cond_value value, ast_node *left, ast_node *right) {
	ast_value astv = { .cond = value };
	return ast_make_node(AST_KIND_COND, astv, left, right);
}

ast_node *ast_make_rdr(ast_rdr_value value, ast_node *left, ast_node *right) {
	ast_value astv = { .rdr = value };
	return ast_make_node(AST_KIND_RDR, astv, left, right);
}

ast_node *ast_make_run(ast_run_value value, ast_node *left, ast_node *right) {
	ast_value astv = { .run = value };
	return ast_make_node(AST_KIND_RUN, astv, left, right);
}

ast_node *ast_make_fdnum(int value) {
	ast_value astv = { .fdnum = value };
	return ast_make_node(AST_KIND_FDNUM, astv, NULL, NULL);
}

ast_node *ast_make_pipe(ast_node *left, ast_node *right) {
	return ast_make_noval_node(AST_KIND_PIPE, left, right);
}

ast_node *ast_make_join(ast_node *left, ast_node *right) {
	return ast_make_noval_node(AST_KIND_JOIN, left, right);
}

ast_node *ast_strdup(ast_kind kind, const char *value) {
	ast_value val = { .str = strdup(value) };
	return ast_make_node(kind, val, NULL, NULL);
}

void ast_recurse_free(ast_node *node) {
	if (node == NULL) {
		return;
	}

	ast_recurse_free(node->left);
	ast_recurse_free(node->right);

	switch (node->kind) {
	case AST_KIND_WORD: // fallthrough
	case AST_KIND_ASSIGN:
		free(node->value.str);
		break;
	default:
		break;
	}

	free(node);
}

/**
 * @brief Create a new AST node with a value.
 *
 * @param[in] kind - Node kind.
 * @param[in] value - Node value.
 * @param[in] left - Left node.
 * @param[in] right - Right node.
 * @return New AST node.
 */
static ast_node *ast_make_node(
		ast_kind kind,
		ast_value value,
		ast_node *left,
		ast_node *right) {
	ast_node *node = malloc(sizeof(ast_node));

	node->kind = kind;
	node->left = left;
	node->right = right;
	node->value = value;

	return node;
}

/**
 * @brief Create a new AST node without a value.
 *
 * @param[in] kind - Node kind.
 * @param[in] left - Left node.
 * @param[in] right - Right node.
 * @return New AST node.
 */
static ast_node *ast_make_noval_node(
		ast_kind kind,
		ast_node *left,
		ast_node *right) {
	ast_node *node = malloc(sizeof(ast_node));

	node->kind = kind;
	node->left = left;
	node->right = right;

	// Just to leave it initialized
	node->value.str = NULL;

	return node;
}

