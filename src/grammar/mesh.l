%{
	#include "ast.h"
	#include "y.tab.h"

	// Missing special characters
	// This does not handle escaped quotes
%}

digit [0-9]
alpha [a-zA-z]
symbol [-\.\+\$\/]
separator [ \t]
escaped \\.

single_quoted '({escaped}|[^'\\])*'
double_quoted \"({escaped}|[^"\\])*\"

alphanum {digit}|{alpha}
word {alphanum}|{symbol}|{single_quoted}|{double_quoted}|{escaped}

%%

'.*' {
	yylval.node = ast_strdup(AST_KIND_WORD, yytext);
	return WORD;
}
\".*\" {
	yylval.node = ast_strdup(AST_KIND_WORD, yytext);
	return WORD;
}

; { return LS_SEQ; }

\|\| { return LS_OR; }
\| { return PL_PIPE; }

&& { return LS_AND; }
& { return LS_ASYNC; }

\>\> { return RO_APPEND; }
\>\| { return RO_CLOBBER; }
\>& { return RO_DUP; }
\> { return RO_NORMAL; }

\<\> { return RI_IO; }
\<& { return RI_DUP; }
\< { return RI_NORMAL; }

{digit}+/[\<\>] {
	yylval.node = ast_make_fdnum(atoi(yytext));
	return FDNUM;
}

{alpha}{alphanum}*={word}+ {
	yylval.node = ast_strdup(AST_KIND_ASSIGN, yytext);
	return ASSIGNMENT;
}

{word}+ {
	yylval.node = ast_strdup(AST_KIND_WORD, yytext);
	return WORD;
}

\n+ { return NEWLINES; }
{separator}+ /* Ignore */

. { return LEX_ERROR; }

%%

int yywrap() {
	return 1;
}
