#include <stdio.h>

#include "vars.h"

int main() {
	extern const char **environ;
	vars_import(environ);
	vars_print_all(1);

	vars_set("X", "5");
	vars_set("Y", "6");

	vars_print_all(0);

	printf("%s\n", vars_get("X"));
	printf("%s\n", vars_get("Y"));

	return 0;
}
