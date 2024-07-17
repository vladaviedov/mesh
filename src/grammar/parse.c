#include "parse.h"

#include <stdlib.h>

#include "ast.h"
#include "../util/helper.h"

typedef void * YY_BUFFER_STATE;

extern int yyparse(ast_node **root);
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

ast_node *parse_from_string(const char *str) {
	YY_BUFFER_STATE buffer = yy_scan_string(str);

	ast_node *root = NULL;
	int result = yyparse(&root);
	if (result != 0) {
		print_error("failed to parse input");
		return NULL;
	}

	yy_delete_buffer(buffer);
	return root;
}
