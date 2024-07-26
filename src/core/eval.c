#include "eval.h"

#include <stdlib.h>

#include <c-utils/vector.h>
#include <c-utils/stack.h>

#include "../util/helper.h"
#include "../core/run.h"
#include "../grammar/ast.h"
#include "../grammar/expand.h"

// Track quote state
typedef enum {
	Q_NONE,
	Q_SINGLE,
	Q_DOUBLE
} quote_st;

static int eval_seq(ast_node *seq, int carry_async);

static int eval_child(ast_node *child, run_flags *flags);
static int eval_cond(ast_node *cond, run_flags *flags);
static int eval_pipe(ast_node *pipe, run_flags *flags);
static int eval_run(ast_node *run, run_flags *flags);

static string_vector *to_argv(ast_node *target);
static void add_word_to_argv(ast_node *word, string_vector *argv);

int eval_ast(ast_node *root) {
	if (root->kind == AST_KIND_SEQ) {
		return eval_seq(root, 0);
	}

	run_flags empty_set = {
		.redirs = vec_init(sizeof(redir)),
		.assigns = vec_init(sizeof(assign)),
	};

	return eval_child(root, &empty_set);
}

/* void eval_pre_process(ast_node *node) { */
/* 	if (node == NULL) { */
/* 		return; */
/* 	} */

/* 	// Process AST body to argv */
/* 	if (node->kind == AST_KIND_RUN && node->value.run == AST_RUN_EXECUTE) { */
/* 		ast_node *exec_root = node->left; */
/* 		node->left = ast_make_argv(to_argv(exec_root)); */
/* 		ast_recurse_free(exec_root); */
/* 		return; */
/* 	} */

/* 	// Other strings: only expand */
/* 	if (node->kind == AST_KIND_WORD || node->kind == AST_KIND_ASSIGN) { */
/* 		char *value = node->value.str; */
/* 		node->value.str = expand_word(value); */
/* 		free(value); */
/* 		return; */
/* 	} */

/* 	eval_pre_process(node->left); */
/* 	eval_pre_process(node->right); */
/* } */

static int eval_child(ast_node *child, run_flags *flags) {
	switch (child->kind) {
	case AST_KIND_COND:
		return eval_cond(child, flags);
	case AST_KIND_PIPE:
		return eval_pipe(child, flags);
	case AST_KIND_RUN:
		return eval_run(child, flags);
	default:
		return -1;
	}
}

static int eval_seq(ast_node *seq, int carry_async) {
	run_flags flags = {
		.redirs = vec_init(sizeof(redir)),
		.assigns = vec_init(sizeof(assign)),
	};

	// Left side - required
	if (seq->left->kind == AST_KIND_SEQ) {
		// TODO: implement async eval
		eval_seq(seq->left, seq->value.seq == AST_SEQ_ASYNC);
	} else {
		eval_child(seq->left, &flags);
	}

	// Right side - optional
	if (seq->right != NULL) {
		eval_child(seq->right, &flags);
	}

	return 0;
}

static int eval_cond(ast_node *cond, run_flags *flags) {
	int left_result = eval_child(cond->left, flags);

	if (cond->value.cond == AST_COND_AND && left_result == 0) {
		return eval_child(cond->right, flags);
	}
	if (cond->value.cond == AST_COND_OR && left_result != 0) {
		return eval_child(cond->right, flags);
	}

	return left_result;
}

static int eval_pipe(ast_node *pipe, run_flags *flags) {
	// TODO: implement
	print_warning("pipes not implemented\n");
	return 0;
}

static int eval_run(ast_node *run, run_flags *flags) {
	ast_run_value type = run->value.run;

	if (type == AST_RUN_APPLY) {
		print_warning("apply not implemented\n");
		return 0;
	}
	if (type == AST_RUN_SHELL_ENV) {
		print_warning("shell env not implemented\n");
		return 0;
	}

	string_vector *argv = to_argv(run->left);
	return run_dispatch(argv, flags);
}

static string_vector *to_argv(ast_node *target) {
	stack words = stack_init(sizeof(ast_node));

	// Parser will always build the tree to the left
	while (target->kind == AST_KIND_JOIN) {
		stack_push(&words, target->right);
		target = target->left;
	}

	// Left-most node
	stack_push(&words, target);
	
	// Now process to argv
	string_vector *argv = vec_new(sizeof(char *));
	ast_node buffer;
	while (stack_pop(&words, &buffer) != STACK_STATUS_EMPTY) {
		add_word_to_argv(&buffer, argv);
	}

	stack_deinit(&words);
	return argv;
}

static void add_word_to_argv(ast_node *word, string_vector *argv) {
	char *expanded = expand_word(word->value.str);

	char_vector final_arg = vec_init(sizeof(char));

	char *trav = expanded;
	char ch;
	quote_st quotes = Q_NONE;

	while ((ch = *trav++) != '\0') {
		switch (ch) {
		case '\\':
			ch = *trav++;
			vec_push(&final_arg, &ch);
			break;
		case ' ': // fallthrough
		case '\t':
			if (final_arg.count == 0) {
				break;
			}

			if (quotes == Q_NONE) {
				vec_push(&final_arg, &null_char);
				char *arg = vec_collect(&final_arg);
				vec_push(argv, &arg);
			} else {
				vec_push(&final_arg, &ch);
			}
			break;
		case '\"':
			switch (quotes) {
			case Q_NONE:
				quotes = Q_DOUBLE;
				break;
			case Q_DOUBLE:
				quotes = Q_NONE;
				break;
			case Q_SINGLE:
				vec_push(&final_arg, &ch);
				break;
			}
			break;
		case '\'':
			switch (quotes) {
			case Q_NONE:
				quotes = Q_SINGLE;
				break;
			case Q_SINGLE:
				quotes = Q_NONE;
				break;
			case Q_DOUBLE:
				vec_push(&final_arg, &ch);
				break;
			}
			break;
		default:
			vec_push(&final_arg, &ch);
			break;
		}
	}

	if (final_arg.count != 0) {
		vec_push(&final_arg, &null_char);
		char *arg = vec_collect(&final_arg);
		vec_push(argv, &arg);
	}

	free(expanded);
}
