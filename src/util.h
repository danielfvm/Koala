#include <stddef.h>

#define bool char

#define true  1
#define false 0
#define NOT_FOUND -1

bool   kl_util_has_illigal_ascii (const char* text);

char*  kl_util_read_file (const char* filepath);

int    kl_util_find_next_bracket (size_t p, const char* text);

int    kl_util_find_string_end (size_t p, const char* text);

char*  kl_util_substr (const char *src, int m, int n);

void   kl_util_trim (char** text);

void   kl_util_ctrim (char** text);

bool   kl_util_is_bracket (char c);

size_t kl_util_contains (char* text, char c);

void   kl_util_filter_comment (char** text);

bool   kl_util_is_str_concat (char* str);

int    kl_util_split (char* buffer, char delim, char*** output);

char*  kl_util_str_replace (char* orig, char* rep, char* with);
