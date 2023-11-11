#include "vars.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "vector.h"

typedef struct {
	char *key;
	char *value;
	int is_export;
} sh_var;
typedef vector sh_var_vector;

// Env var vector
static sh_var_vector *env_vars = NULL;
// Export counter
static uint32_t export_count = 0;

char *sh_var_to_string(const sh_var *var);

void vars_import(const char **env) {
	// Reset env vars if populated
	if (env_vars != NULL) {
		vec_free(env_vars);
		export_count = 0;
	}

	env_vars = vec_new(sizeof(sh_var));
	while (*env != NULL) {
		char *copy = strdup(*env);

		sh_var var = {
			.key = strtok(copy, "="),
			.value = strtok(NULL, "="),
			.is_export = 1
		};
		vec_push(env_vars, &var);

		env++;
		export_count++;
	}
}

char **vars_export(void) {
	if (env_vars == NULL) {
		return NULL;
	}

	char **env = ntmalloc(export_count, sizeof(char **));
	uint32_t index = 0;

	for (uint32_t i = 0; i < env_vars->count; i++) {
		sh_var *entry = vec_at(env_vars, i);
		if (entry == NULL) {
			return NULL;
		}

		if (entry->is_export) {
			env[index] = sh_var_to_string(entry);
			index++;
		}
	}

	return env;
}

// TODO: implement
int vars_set(const char *key, const char *value);
char *vars_get(const char *key);
int vars_delete(const char *key);
int vars_set_export(const char *key);

void vars_print_all(int export_flag) {
	if (env_vars == NULL) {
		return;
	}

	for (uint32_t i = 0; i < env_vars->count; i++) {
		sh_var *entry = vec_at(env_vars, i);

		// If export is set, skip unexported
		if (export_flag && !entry->is_export) {
			continue;
		}

		// Export prefix
		if (export_flag) {
			printf("export ");
		}

		printf("%s=%s\n", entry->key, entry->value);
	}
}

/** Internal */

/**
 * @brief Convert internal variable to export format.
 *
 * @param[in] var - Internal variable struct.
 * @return String representation; NULL on error.
 * @note Allocated return value.
 */
char *sh_var_to_string(const sh_var *var) {
	size_t str_len = strlen(var->key) + strlen(var->value) + 1;
	char *str = ntmalloc(str_len, sizeof(char));
	
	if (snprintf(str, str_len + 1, "%s=%s", var->key, var->value) < 0) {
		return NULL;
	}

	return str;
}
