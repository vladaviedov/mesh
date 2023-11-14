#include <stdio.h>
#include <stdlib.h>

#include "vector.h"
#include "vars.h"
#include "parser.h"
#include "run.h"

int main() {
	extern const char **environ;
	vars_import(environ);

	char buffer[1024] = {0};
	uint32_t index = 0;

	printf("$ ");
	int ch;
	while ((ch = fgetc(stdin)) != '\n' && ch != EOF) {
		buffer[index] = ch;
		index++;
	}

	char *subbed_str = parser_sub(buffer);
	str_vec *parsed_str = parser_split(subbed_str);
	for (uint32_t i = 0; i < parsed_str->count; i++) {
		printf("%u: %s\n", i, *(char **)vec_at(parsed_str, i));
	}

	run_dispatch(parsed_str);

	vec_free_with_elements(parsed_str);
	free(subbed_str);

	return 0;
}
