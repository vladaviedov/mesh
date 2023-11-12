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
sh_var *find_sh_var(const char *key, uint32_t *index);

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
			.key = strdup(strtok(copy, "=")),
			.value = strdup(strtok(NULL, "=")),
			.is_export = 1
		};

		free(copy);
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

void vars_set(const char *key, const char *value) {
	sh_var *find_res = find_sh_var(key, NULL);

	if (find_res == NULL) {
		// Create new variable
		sh_var new_var = {
			.key = strdup(key),
			.value = strdup(value),
			.is_export = 0
		};
		vec_push(env_vars, &new_var);
	} else {
		// Update existing
		free(find_res->value);
		find_res->value = strdup(value);
	}
}

const char *vars_get(const char *key) {
	sh_var *find_res = find_sh_var(key, NULL);
	if (find_res == NULL) {
		return NULL;
	}

	return find_res->value;
}

int vars_delete(const char *key) {
	uint32_t index;
	sh_var *find_res = find_sh_var(key, &index);
	if (find_res == NULL) {
		return -1;
	}

	return vec_pop(env_vars, index, NULL);
}

int vars_set_export(const char *key) {
	sh_var *find_res = find_sh_var(key, NULL);
	if (find_res == NULL) {
		return -1;
	}

	find_res->is_export = 1;
	return 0;
}

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

/**
 * @brief Find shell variable entry by name.
 *
 * @param[in] key - Variable name.
 * @param[out] index - If not NULL and item was found, places index here.
 * @return Shell variable structure; NULL on error.
 */
sh_var *find_sh_var(const char *key, uint32_t *index) {
	for (uint32_t i = 0; i < env_vars->count; i++) {
		sh_var *entry = vec_at(env_vars, i);
		if (strcmp(entry->key, key) == 0) {
			if (index != NULL) {
				*index = i;
			}
			return entry;
		}
	}

	return NULL;
}
