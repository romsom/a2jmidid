#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "format.h"

format_t g_a2j_jack_port_format;
char * g_a2j_jack_port_format_keys[] = {
	"client_name",
	"client_id",
	"port_name",
	"port_id",
	"port_type"
};
const size_t g_a2j_jack_port_format_keys_cnt = sizeof(g_a2j_jack_port_format_keys) / sizeof(g_a2j_jack_port_format_keys[0]);

struct format {
	char * format;
	size_t n_elements;
	ssize_t * replacement_indices;
	char ** format_slices;
	int n_invalid_replacements;
};

format_t
parse_format(char * format_str, char * replacements[], size_t n_replacements) {
	struct format * format = calloc(1, sizeof(*format));
	format->format = strdup(format_str);
	size_t n_upper_limit = strlen(format_str) + 1; // reserve at least one element
	char * format_slices[n_upper_limit];
	ssize_t replacement_indices[n_upper_limit];
	bool is_replacement = false;

	// next points to '%' or '\0' after replacement or is NULL
	char *next = strpbrk(format->format, "%");
	if (next != format->format) { // including NULL
		format_slices[0] = format->format;
		replacement_indices[0] = -1; // no replacement
	} else if (strlen(format->format) > 0) { // format starts with replacement
		is_replacement = true;
		format_slices[0] = next + 1;
		next = strpbrk(format_slices[0], "%");
	} // else empty string

	// terminate format_slices[cur_element]
	if (next != NULL) {
		*next = '\0';
	}

	int err = 0;
	size_t cur_element = 0;
	while (cur_element < n_upper_limit)
	{
		if (strlen(format_slices[cur_element]) > 0) {
			if (is_replacement) {
				// find replacement string
				size_t replacement_idx = n_replacements;
				for (ssize_t i = 0; (size_t)i < n_replacements; i++) {
					if (strcmp(format_slices[cur_element], replacements[i]) == 0) { // found replacement
						replacement_idx = i;
						break;
					}
				}
				if (replacement_idx == n_replacements) // found no replacement
					err += 1;

				replacement_indices[cur_element] = replacement_idx;
			} else {
				replacement_indices[cur_element] = -1;
			}
		}

		is_replacement = !is_replacement;
		if (strlen(format_slices[cur_element]) > 0)
			cur_element += 1;
		if (next == NULL)
			break;
		// there is always at least an extra NULL byte
		format_slices[cur_element] = next + 1;
		next = strpbrk(format_slices[cur_element], "%");
		// terminate format_slices[cur_element]
		if (next != NULL) {
			*next = '\0';
		}
	}

	format->n_invalid_replacements = err;
	size_t n_elements = cur_element;
	format->n_elements = n_elements;

	format->format_slices = malloc(n_elements * sizeof(*format->format_slices));
	memcpy(format->format_slices, format_slices, n_elements * sizeof(*format->format_slices));
	format->replacement_indices = malloc(n_elements * sizeof(*format->replacement_indices));
	memcpy(format->replacement_indices, replacement_indices, n_elements * sizeof(*format->replacement_indices));

	return format;
}

int
print_format(format_t format, const char * replacements[], size_t n_replacements, char **dest)
{
	int err = 0;

	size_t buffer_size = 1; // reserve space for \0
	for (size_t elm=0; elm<format->n_elements; elm++) {
		if (format->replacement_indices[elm] < 0) {
			buffer_size += strlen(format->format_slices[elm]);
		} else if ((size_t)(format->replacement_indices[elm]) < n_replacements) {
			buffer_size += strlen(replacements[format->replacement_indices[elm]]);
		} // ignore invalid replacements
	}

	char *res = calloc(buffer_size, sizeof(*res));
	char *pos = res;

	for (size_t elm=0; elm<format->n_elements; elm++) {
		if (format->replacement_indices[elm] < 0) {
			pos = mempcpy(pos, format->format_slices[elm], strlen(format->format_slices[elm]));
		} else if ((size_t)(format->replacement_indices[elm]) < n_replacements) {
			const char *replacement = replacements[format->replacement_indices[elm]];
			pos = mempcpy(pos, replacement, strlen(replacement));
		} // ignore invalid replacements
	}

	// assert(pos == res + buffer_size - 1);

	*dest = res;
	
	return err;
}

void free_format(format_t format) {
	if (format) {
		if (format->replacement_indices)
			free(format->replacement_indices);
		if (format->format_slices)
			free(format->format_slices);
		if (format->format)
			free(format->format);
		free(format);
	}
}

int get_invalid_pattern_count(format_t format) {
	return format->n_invalid_replacements;
}
