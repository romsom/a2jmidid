#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct format * format_t;
extern format_t g_a2j_jack_port_format;
extern char * g_a2j_jack_port_format_keys[];
extern const size_t g_a2j_jack_port_format_keys_cnt;

format_t parse_format(char * format_str, char * replacements[], size_t n_replacements);
int get_invalid_pattern_count(format_t format);
int print_format(format_t format, const char * replacements[], size_t n_replacements, char **dest);
void free_format(format_t format);
