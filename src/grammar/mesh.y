%{
%}

// Primary tokens
%token NUMBER
%token ASSIGN
%token WORD
%token NEWLINES
%token ASSIGNMENT

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

%start program

%%

program: break list break
	   | break
	   ;

list: list separator cond_list
	| list separator
	| cond_list
	;

cond_list: cond_list LS_AND break unit
		 | cond_list LS_OR break unit
		 | unit
		 ;

unit: unit PL_PIPE break command
	| command
	;

command: prefix body redirect_list
	   | prefix body
	   | prefix
	   | body redirect_list
	   | body
	   ;

prefix: prefix ASSIGNMENT
	  | prefix redirect
	  | ASSIGNMENT
	  | redirect
	  ;

body: body WORD
	| WORD
	;

redirect_list: redirect_list redirect
			 | redirect
			 ;

redirect: NUMBER redirect_op WORD
		| redirect_op WORD
		;

redirect_op: RO_NORMAL
		   | RO_CLOBBER
		   | RO_APPEND
		   | RO_DUP
		   | RI_NORMAL
		   | RI_DUP
		   | RI_IO
		   ;

break: NEWLINES
	 | // epsilon
	 ;

separator: LS_SEQ
		 | LS_ASYNC
		 ;

%%
