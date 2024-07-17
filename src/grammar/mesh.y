%{
	#include "ast.h"

	extern char *yytext;
%}

%union {
	ast_node *node;
	ast_value value;
}

%parse-param {ast_node **root}

// Primary tokens
%token <node> NUMBER
%token <node> WORD
%token <node> ASSIGNMENT
%token NEWLINES

// sh: List operators
%token LS_SEQ
%token LS_AND
%token LS_OR
%token LS_ASYNC

// sh: Output redirections
%token RO_NORMAL
%token RO_CLOBBER
%token RO_APPEND
%token RO_DUP

// sh: Input redirections
%token RI_NORMAL
%token RI_DUP
%token RI_IO
// %token RI_HEREDOC
// %token RI_HEREDOC_NOTAB

// sh: Expansions
// %token EX_PARAM
// %token EX_HOME
// %token EX_GLOB

// sh: Pipelines
%token PL_PIPE

// Non-terminal types
%type <node> program
%type <node> list
%type <node> cond_list
%type <node> unit
%type <node> command
%type <node> prefix
%type <node> body
%type <node> redirect_list
%type <node> redirect
%type <value> redirect_op
%type <value> separator

%start program

%%

program: break list break { *root = $2; }
	   | break { *root = NULL; }
	   ;

list: list separator cond_list {
	$$ = ast_make_node(AST_KIND_SEQ, $2, $1, $3);
}
	| list separator {
	$$ = ast_make_node(AST_KIND_SEQ, $2, $1, NULL);
}
	| cond_list { $$ = $1; }
	;

cond_list: cond_list LS_AND break unit {
	$$ = ast_make_node(AST_KIND_COND, AST_COND_AND, $1, $4);
}
		 | cond_list LS_OR break unit {
	$$ = ast_make_node(AST_KIND_COND, AST_COND_OR, $1, $4);
}
		 | unit { $$ = $1; }
		 ;

unit: unit PL_PIPE break command {
	$$ = ast_make_noval_node(AST_KIND_PIPE, $1, $4);
}
	| command { $$ = $1; }
	;

command: prefix body redirect_list {
	$$ = ast_make_node(AST_KIND_RUN, AST_RUN_APPLY,
		$1,
		ast_make_node(AST_KIND_RUN, AST_RUN_APPLY,
			$3,
			ast_make_node(AST_KIND_RUN, AST_RUN_EXECUTE, $2, NULL)
		)
	);
}
	   | prefix body {
	$$ = ast_make_node(AST_KIND_RUN, AST_RUN_APPLY,
		$1,
		ast_make_node(AST_KIND_RUN, AST_RUN_EXECUTE, $2, NULL)
	);
}
	   | prefix {
	$$ = ast_make_node(AST_KIND_RUN, AST_RUN_SHELL_ENV, $1, NULL);
}
	   | body redirect_list {
	$$ = ast_make_node(AST_KIND_RUN, AST_RUN_APPLY,
		$2,
		ast_make_node(AST_KIND_RUN, AST_RUN_EXECUTE, $1, NULL)
	);
}
	   | body {
	$$ = ast_make_node(AST_KIND_RUN, AST_RUN_EXECUTE, $1, NULL);
}
	   ;

prefix: prefix ASSIGNMENT {
	$$ = ast_make_noval_node(AST_KIND_JOIN, $1, $2);
}
	  | prefix redirect {
	$$ = ast_make_noval_node(AST_KIND_JOIN, $1, $2);
}
	  | ASSIGNMENT { $$ = $1; }
	  | redirect { $$ = $1; }
	  ;

body: body WORD {
	$$ = ast_make_noval_node(AST_KIND_JOIN, $1, $2);
}
	| WORD { $$ = $1; }
	;

redirect_list: redirect_list redirect {
	$$ = ast_make_noval_node(AST_KIND_JOIN, $1, $2);
}
			 | redirect { $$ = $1; }
			 ;

redirect: NUMBER redirect_op WORD {
	$$ = ast_make_node(AST_KIND_RDR, $2, $1, $3);
}
		| redirect_op WORD {
	$$ = ast_make_node(AST_KIND_RDR, $1, 1, $2);
}
		;

redirect_op: RO_NORMAL { $$.rdr = AST_RDR_O_NORMAL; }
		   | RO_CLOBBER { $$.rdr = AST_RDR_O_CLOBBER; }
		   | RO_APPEND { $$.rdr = AST_RDR_O_APPEND; }
		   | RO_DUP { $$.rdr = AST_RDR_O_DUP; }
		   | RI_NORMAL { $$.rdr = AST_RDR_I_NORMAL; }
		   | RI_DUP { $$.rdr = AST_RDR_I_DUP; }
		   | RI_IO { $$.rdr = AST_RDR_I_IO; }
		   ;

break: NEWLINES
	 | // epsilon
	 ;

separator: LS_SEQ { $$.seq = AST_SEQ_NORMAL; }
		 | LS_ASYNC { $$.seq = AST_SEQ_ASYNC; }
		 ;

%%
