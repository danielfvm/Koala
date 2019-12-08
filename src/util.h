#include <stddef.h>

#define bool char

#define true  1
#define false 0

bool   frs_has_illigal_ascii (const char* text);

int    frs_find_next_bracket (size_t p, const char* text);

char*  frs_substr (const char *src, int m, int n);

void   frs_trim (char** text);

void   frs_ctrim (char** text);

bool   frs_is_bracket (char c);

size_t frs_contains (char* text, char c);

bool   frs_is_str_concat (char* str);

int    frs_split (char* buffer, char delim, char*** output);

char*  frs_str_replace (char* orig, char* rep, char* with);
