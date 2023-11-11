#include <stdio.h>

#include "vars.h"

int main() {
	extern const char **environ;
	vars_import(environ);
	vars_print_all(1);

	return 0;
}
