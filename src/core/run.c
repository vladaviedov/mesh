#include "run.h"

#include <stdlib.h>
#include <string.h>

#include "../util/helper.h"
#include "../ext/meta.h"
#include "vars.h"
#include "parser.h"
#include "builtins.h"
#include "exec.h"

int run_dispatch(str_vec *args) {
	// Meta commands
	char *argv0 = fix_ptr(vec_at(args, 0));
	if (*argv0 == ':') {
		char *meta_out;
		int meta_result = run_meta(args, &meta_out);

		if (meta_result < 0) {
			return -1;
		}
		if (meta_result == 0) {
			return 0;
		}

		char *subbed_str = parser_sub(meta_out);

		char *end = subbed_str;
		int result;
		do {
			str_vec *parsed_str = parser_split(end, &end);
			result = run_dispatch(parsed_str);
			vec_free_with_elements(parsed_str);
		} while (end != NULL);

		free(subbed_str);
		free(meta_out);
		return result;
	}

	// Shell assignments
	if (is_pure_assign(args)) {
		return pure_assign(args->count, args->raw, 0);
	}
	
	// Builtins
	int code = run_builtin(args);
	if (code >= 0) {
		return code;
	}
	
	// Add null-terminator to args
	char *argv[args->count + 1];
	memcpy(argv, args->raw, args->count * sizeof(char *));
	argv[args->count] = NULL;

	// Exec program
	return exec_normal(argv);
}
