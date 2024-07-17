#pragma once

typedef enum {
	// Enum value
	AST_KIND_SEQ,
	AST_KIND_COND,
	AST_KIND_RDR,
	AST_KIND_RUN,
	// Literal value
	AST_KIND_WORD,
	AST_KIND_FDNUM,
	AST_KIND_ASSIGN,
	// No value
	AST_KIND_PIPE,
	AST_KIND_JOIN,
} ast_kind;

typedef enum {
	AST_SEQ_NORMAL,
	AST_SEQ_ASYNC,
} ast_seq_value;

typedef enum {
	AST_COND_AND,
	AST_COND_OR,
} ast_cond_value;

typedef enum {
	AST_RDR_O_NORMAL,
	AST_RDR_O_CLOBBER,
	AST_RDR_O_APPEND,
	AST_RDR_O_DUP,
	AST_RDR_I_NORMAL,
	AST_RDR_I_DUP,
	AST_RDR_I_IO,
} ast_rdr_value;

typedef enum {
	AST_RUN_APPLY,
	AST_RUN_EXECUTE,
	AST_RUN_SHELL_ENV,
} ast_run_value;

typedef union {
	ast_seq_value seq;
	ast_cond_value cond;
	ast_rdr_value rdr;
    ast_run_value run;

	char *str;
	int fdnum;
} ast_value;

typedef struct ast_node {
	ast_kind kind;

	struct ast_node *left;
	struct ast_node *right;

	ast_value value;
} ast_node;

ast_node *ast_make_seq(ast_seq_value value, ast_node *left, ast_node *right);
ast_node *ast_make_cond(ast_cond_value value, ast_node *left, ast_node *right);
ast_node *ast_make_rdr(ast_rdr_value value, ast_node *left, ast_node *right);
ast_node *ast_make_run(ast_run_value value, ast_node *left, ast_node *right);
ast_node *ast_make_fdnum(int value);
ast_node *ast_make_pipe(ast_node *left, ast_node *right);
ast_node *ast_make_join(ast_node *left, ast_node *right);

ast_node *ast_strdup(ast_kind kind, const char *value);

void ast_recurse_free(ast_node *node);
