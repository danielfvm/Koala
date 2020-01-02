#include <stddef.h>

/* Bool DataType does not exist in STD-C */
#define bool char

/* Defining true and false for better overview */
#define true  1
#define false 0

/* Used in ´kl_util_find_next_bracket´ if nothing was found */
#define NOT_FOUND -1

/* Checks if str contains not ´0..9 A..Z a..z _ .´ */
bool   kl_util_has_illigal_ascii (const char* text);

/* Removes first & last char */
void   kl_util_trim_front_end (char** str);

/* Returns file content as string */
char*  kl_util_read_file (const char* filepath);

/* Finds next bracket independent of brackets in between */
int    kl_util_find_next_bracket (size_t p, const char* text);

/* Seaches the end of an string ´"´ */
int    kl_util_find_string_end (size_t p, const char* text);

/* Creates a substring of an existing string, dont forget to free! */
char*  kl_util_substr (const char *src, int m, int n);

/* Trims all spaces/tabs/endl from beginn & end of string */
void   kl_util_trim (char** text);

/* Trim all spaces/tabs/endl in whole string */
void   kl_util_ctrim (char** text);

/* Check if char is a valid bracket */
bool   kl_util_is_bracket (char c);

/* Check if string contains char & returning its index + 1 */
size_t kl_util_contains (char* text, char c);

/* Removes all comments from code ´<* *>´ ´**´ */
void   kl_util_filter_comment (char** text);

/* Check if string contains a string concat syntax */
bool   kl_util_is_str_concat (char* str);

/* Splits a string using a char into an array */
int    kl_util_split (char* buffer, char delim, char*** output);

/* Replaces certain strings with another string */
char*  kl_util_str_replace (char* orig, char* rep, char* with);
