#include "ast.h"

#include <stdlib.h>
#include <string.h>

ast_node *ast_make_node(
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

ast_node *ast_make_noval_node(
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

ast_node *ast_strdup(
		ast_kind kind,
		const char *value) {
	ast_value val = {
		.str = strdup(value),
	};

	return ast_make_node(
			kind,
			val,
			NULL,
			NULL);
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
