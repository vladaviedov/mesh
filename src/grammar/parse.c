/**
 * @file grammar/parse.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief YACC parser interface.
 */
#define _POSIX_C_SOURCE 200809L
#include "parse.h"

#include <stdlib.h>

#include "../util/error.h"
#include "ast.h"

// Needed to interface with Flex & Yacc
typedef void *YY_BUFFER_STATE;
extern int yyparse(ast_node **root);
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

ast_node *parse_from_string(const char *str) {
	YY_BUFFER_STATE buffer = yy_scan_string(str);

	ast_node *root = NULL;
	int result = yyparse(&root);

	yy_delete_buffer(buffer);

	if (result != 0) {
		print_error("syntax error\n");
		return NULL;
	}

	return root;
}
