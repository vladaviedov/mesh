/**
 * @file core/eval.c
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version 0.3.0
 * @date 2024
 * @license GPLv3.0
 * @brief AST evaluation and argv parsing.
 */
#define _POSIX_C_SOURCE 200809L
#include "eval.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <c-utils/stack.h>
#include <c-utils/vector.h>

#include "../core/run.h"
#include "../grammar/ast.h"
#include "../grammar/expand.h"
#include "../util/helper.h"

// Track quote state
typedef enum {
	Q_NONE,
	Q_SINGLE,
	Q_DOUBLE
} quote_st;

static int eval_seq(ast_node *seq, int carry_async);

static int eval_child(ast_node *child, run_flags *flags);
static int eval_cond(ast_node *cond, run_flags *flags);
static int eval_pipe(ast_node *pipeline, run_flags *flags);
static int eval_run(ast_node *run, run_flags *flags);

static string_vector *to_argv(ast_node *target);
static void add_word_to_argv(ast_node *word, string_vector *argv);
static void to_flags(ast_node *apply, run_flags *flags);
static void add_redir_to_flags(ast_node *rdr, run_flags *flags);
static void add_assign_to_flags(ast_node *node, run_flags *flags);

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

/**
 * @brief Evaluate child structure AST node (auto select by type).
 *
 * @param[in] child - Child node.
 * @param[in] flags - Run flags.
 * @return Evaluation result.
 */
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

/**
 * @brief Evaluate sequence node.
 *
 * @param[in] seq - Sequence node.
 * @param[in] carry_async - Whether the right child should be run async.
 * @return Evaluation result.
 */
static int eval_seq(ast_node *seq, int carry_async) {
	run_flags flags = {
		.redirs = vec_init(sizeof(redir)),
		.assigns = vec_init(sizeof(assign)),
	};

	int result;

	// Left side - required
	if (seq->left->kind == AST_KIND_SEQ) {
		// TODO: implement async eval
		result = eval_seq(seq->left, seq->value.seq == AST_SEQ_ASYNC);
	} else {
		result = eval_child(seq->left, &flags);
	}

	// Right side - optional
	if (seq->right != NULL) {
		result = eval_child(seq->right, &flags);
	}

	del_flags(&flags);
	return result;
}

/**
 * @brief Evaluate conditional list node.
 *
 * @param[in] cond - Conditional list.
 * @param[in] flags - Run flags.
 * @return Evaluation result.
 */
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

/**
 * @brief Evaluate command with pipes.
 *
 * @param[in] pipeline - Pipe node.
 * @param[in] flags - Run flags.
 * @return Evaluation result.
 */
static int eval_pipe(ast_node *pipeline, run_flags *flags) {
	int pipe_fds[2];
	if (pipe(pipe_fds) < 0) {
		return -1;
	}

	// Redirect left commmand's output to the write end
	run_flags left_flags = copy_flags(flags);
	redir output = {
		.type = RDR_FD,
		.flags = 0,
		.from = STDOUT_FILENO,
		.to.fd = pipe_fds[1],
	};
	vec_push(&left_flags.redirs, &output);
	eval_child(pipeline->left, &left_flags);
	close(pipe_fds[1]);
	del_flags(&left_flags);

	// Redirect read end to right command's input
	run_flags right_flags = copy_flags(flags);
	redir input = {
		.type = RDR_FD,
		.flags = 0,
		.from = STDIN_FILENO,
		.to.fd = pipe_fds[0],
	};
	vec_push(&right_flags.redirs, &input);

	int result = eval_child(pipeline->right, &right_flags);
	close(pipe_fds[0]);
	del_flags(&right_flags);
	return result;
}

/**
 * @brief Evaluate runnable AST nodes.
 *
 * @param[in] run - Runnable node.
 * @param[in] flags - Run flags.
 * @return Evaluation result.
 */
static int eval_run(ast_node *run, run_flags *flags) {
	ast_run_value type = run->value.run;

	if (type == AST_RUN_APPLY) {
		to_flags(run->left, flags);
		return eval_run(run->right, flags);
	}
	if (type == AST_RUN_SHELL_ENV) {
		print_warning("shell env not implemented\n");
		return 0;
	}

	string_vector *argv = to_argv(run->left);
	int result = run_dispatch(argv, flags);
	free_elements(argv);
	vec_delete(argv);

	return result;
}

/**
 * @brief Convert a command body into an argument vector.
 *
 * @param[in] target - Command body root node.
 * @return Argument vector.
 */
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

/**
 * @brief Extract data from a word AST node into the argument vector.
 *
 * @param[in] word - AST node.
 * @param[in,out] argv - Argument vector being constructed.
 */
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

/**
 * @brief Apply redirections and assignments to flags.
 *
 * @param[in] apply - Apply root node.
 * @param[in,out] flags - Flags to populate.
 */
static void to_flags(ast_node *apply, run_flags *flags) {
	stack words = stack_init(sizeof(ast_node));

	// Parser will always build the tree to the left
	while (apply->kind == AST_KIND_JOIN) {
		stack_push(&words, apply->right);
		apply = apply->left;
	}

	// Left-most node
	stack_push(&words, apply);

	// Now process to flags
	ast_node buffer;
	while (stack_pop(&words, &buffer) != STACK_STATUS_EMPTY) {
		if (buffer.kind == AST_KIND_RDR) {
			add_redir_to_flags(&buffer, flags);
		} else if (buffer.kind == AST_KIND_ASSIGN) {
			add_assign_to_flags(&buffer, flags);
		}
	}

	stack_deinit(&words);
}

static void add_redir_to_flags(ast_node *rdr, run_flags *flags) {
	int fd_from = rdr->left->value.fdnum;
	char *str_to = rdr->right->value.str;

	int fd_to;
	if (rdr->value.rdr == AST_RDR_O_DUP || rdr->value.rdr == AST_RDR_I_DUP) {
		if (str_to[0] == '-') {
			fd_to = -1;
		} else {
			char *end;
			fd_to = strtoul(str_to, &end, 10);
			if (end == str_to) {
				print_warning("invalid duplication target; skipping\n");
				return;
			}
		}
	}

	redir new_redir = {
		.flags = 0,
		.from = fd_from,
	};

	switch (rdr->value.rdr) {
	case AST_RDR_I_DUP:
		if (fd_to < 0) {
			new_redir.type = RDR_CLOSE;
			break;
		}

		new_redir.type = RDR_FD;
		new_redir.to.fd = fd_to;
		if (new_redir.from < 0) {
			new_redir.from = STDIN_FILENO;
		}
		break;
	case AST_RDR_O_DUP:
		if (fd_to < 0) {
			new_redir.type = RDR_CLOSE;
			break;
		}

		new_redir.type = RDR_FD;
		new_redir.to.fd = fd_to;
		if (new_redir.from < 0) {
			new_redir.from = STDOUT_FILENO;
		}
		break;
	case AST_RDR_I_NORMAL:
		new_redir.type = RDR_FILE;
		new_redir.to.filename = str_to;
		new_redir.flags = O_RDONLY;
		if (new_redir.from < 0) {
			new_redir.from = STDIN_FILENO;
		}
		break;
	case AST_RDR_I_IO:
		new_redir.type = RDR_FILE;
		new_redir.to.filename = str_to;
		new_redir.flags = O_RDWR | O_CREAT;
		if (new_redir.from < 0) {
			new_redir.from = STDIN_FILENO;
		}
		break;
	case AST_RDR_O_CLOBBER:
		// TODO: implement properly
	case AST_RDR_O_NORMAL:
		new_redir.type = RDR_FILE;
		new_redir.to.filename = str_to;
		new_redir.flags = O_CREAT | O_WRONLY | O_TRUNC;
		if (new_redir.from < 0) {
			new_redir.from = STDOUT_FILENO;
		}
		break;
	case AST_RDR_O_APPEND:
		new_redir.type = RDR_FILE;
		new_redir.to.filename = str_to;
		new_redir.flags = O_CREAT | O_WRONLY | O_APPEND;
		if (new_redir.from < 0) {
			new_redir.from = STDOUT_FILENO;
		}
		break;
	}

	vec_push(&flags->redirs, &new_redir);
}

static void add_assign_to_flags(ast_node *node, run_flags *flags) {
	char *assign_str = node->value.str;

	char *key = strtok(assign_str, "=");
	char *value = strtok(NULL, "=");

	assign new_assign = {
		.key = key,
		.value = expand_word(value),
	};
	vec_push(&flags->assigns, &new_assign);
}
