#include <stddef.h>

#define bool char

#define true  1
#define false 0
#define NOT_FOUND -1

bool   frs_has_illigal_ascii (const char* text);

char*  frs_read_file (const char* filepath);

int    frs_find_next_bracket (size_t p, const char* text);

int    frs_find_string_end (size_t p, const char* text);

char*  frs_substr (const char *src, int m, int n);

void   frs_trim (char** text);

void   frs_ctrim (char** text);

bool   frs_is_bracket (char c);

size_t frs_contains (char* text, char c);

void   frs_filter_comment (char** text);

bool   frs_is_str_concat (char* str);

int    frs_split (char* buffer, char delim, char*** output);

char*  frs_str_replace (char* orig, char* rep, char* with);
