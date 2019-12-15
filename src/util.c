#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool frs_has_illigal_ascii (const char* text)
{
    for (size_t i = 0; text[i] != '\0'; ++ i)
        if (!((text[i] >= 'a' && text[i] <= 'z') || 
              (text[i] >= 'A' && text[i] <= 'Z') || 
              (text[i] >= '0' && text[i] <= '9') ||
               text[i] == '_' || text[i] == '.'))
                return 1;
    return 0;
}

int frs_find_string_end (size_t p, const char* text)
{
    char string_end;
    int  string_len;
    int  old_p = p + 1;

    if (p >= (string_len = strlen (text)))
    {
        fprintf (stderr, "[CMS][ERR] → first char out of text size\n");
        return old_p;
    }

    string_end = text[p];

    // Add additional brackets here
    if (string_end != '"' && string_end != '\'')
    {
        fprintf (stderr, "[CMS][ERR] → Wrong first char used in ´cms_find_string_end´. First char musst be ´'´, ´\"´");
        return old_p;
    }

    p ++;

    // Searching after closing bracket
    for (; p < string_len; ++ p)
    {
        // Control if this is working!
        if (text[p] == '\\' && (p >= 1 && text[p-1] != '\\') && ++ p)
            continue;

        if (text[p] == string_end && (p <= 0 || text[p-1] != '\\'))
            return p + 1;
    }

    // Closing bracket not found!
    return old_p;
}

int frs_find_next_bracket (size_t p, const char* text)
{
    if (p >= strlen (text))
    {
        fprintf (stderr, "[CMS][ERR] → first char out of text size\n");
        return -1;
    }

    size_t inside_brackets  = 0;
    size_t text_size        = strlen (text);
    char   bracket_open     = text[p];
    char   bracket_close;

    // Add additional brackets here
    switch (bracket_open)
    {
        case '(': bracket_close = ')'; break;
        case '[': bracket_close = ']'; break;
        case '{': bracket_close = '}'; break;
        default:
            fprintf (stderr, "[CMS][ERR] → Wrong first char used in ´cms_find_next_bracket´. First char musst be '(', '[' or '{'!\n");
            return -1;
    }

    int in_string = 0;
    int in_char   = 0;

    // Searching after closing bracket
    for (; p < text_size; ++ p)
    {
        // Control if this is working!
        if (text[p] == '\\' && (p >= 1 && text[p-1] != '\\') && ++ p)
            continue;

        if (text[p] == '\"' && (p <= 0 || text[p-1] != '\\') && !in_char)
            in_string = !in_string;
        if (text[p] == '\'' && (p <= 0 || text[p-1] != '\\') && !in_string)
            in_char = !in_char;

        if (in_string || in_char)
            continue;

        if (text[p] == bracket_open)
            inside_brackets ++;
        if (text[p] == bracket_close)
            inside_brackets --;
        if (inside_brackets <= 0)
            return p;
    }

    // Closing bracket not found!
    return -1;
}

char* frs_substr (const char *src, int m, int n)
{
    unsigned int len = n - m; // length of string
    unsigned int i;

    // allocate (len + 1) chars for destination (+1 for extra null character)
    char *dest = malloc (len + 1);

    // extracts characters between m'th and n'th index from source string
    // and copy them into the destination string
    for (i = m; i < n && (*src != '\0'); ++ i)
    {
        *dest = *(src + i);
        dest ++;
    }

    *dest = '\0';

    return dest - len; // destination string
}

// Trims beginning and end of &string
void frs_trim (char** text)
{
    if (**text == '\0')
        return;

    // Trim begin
    while (**text <= ' ')
        (*text) ++;

    // Trim end
    for (size_t len = strlen (*text) - 1; (*text)[len] <= ' '; -- len)
        (*text)[len] = '\0';
}

void frs_ctrim (char** text)
{
    size_t i, j;
    bool in_string = 0;
    bool in_char   = 0;

    for (i = 0; (*text)[i] != '\0'; ++ i)
    {
        if ((*text)[i] == '\\' && i >= 1 && (*text)[i-1] == '\\' && ++ i)
            continue;

        if ((*text)[i] == '"' && (i == 0 || (*text)[i-1] != '\\') && !in_char)
            in_string = !in_string;

        if ((*text)[i] == '\'' && (i == 0 || (*text)[i-1] != '\\') && !in_string)
            in_char = !in_char;

        if ((*text)[i] > ' ' || in_string || in_char)
            continue;

        for (j = i + 1; (*text)[j] != '\0'; ++ j)
            (*text)[j - 1] = (*text)[j];
        (*text)[j - 1] = '\0';
    }
}

bool frs_is_bracket (char c)
{
    return c == '(' || c == '[' || c == '{' || c == ')' || c == ']' || c == '}';
}

size_t frs_contains (char* text, char c)
{
    bool in_string = 0;
    bool in_char   = 0;
    int in_bracket = 0;

    for (size_t i = 0; text[i] != '\0'; ++ i)
    {
        if (text[i] == '\\' && i >= 1 && text[i-1] == '\\' && ++ i)
            continue;

        if (text[i] == '"' && (i == 0 || text[i-1] != '\\') && !in_char)
            in_string = !in_string;

        if (text[i] == '\'' && (i == 0 || text[i-1] != '\\') && !in_string)
            in_char = !in_char;

        if (text[i] == '(' || text[i] == '[' || text[i] == '{')
            in_bracket ++;
        if (text[i] == ')' || text[i] == ']' || text[i] == '}')
            in_bracket --;

        if (text[i] == c && (frs_is_bracket (c) || (!in_string && !in_char && !in_bracket)))
            return i + 1;
    }
    return 0;
}

void frs_filter_comment (char** text)
{
    size_t i, j;
    bool in_string = false;
    bool in_char   = false;
    int  in_comm   = -1;

    for (i = 0; (*text)[i] != '\0'; ++ i)
    {
        if ((*text)[i] == '\\' && i >= 1 && (*text)[i-1] == '\\' && ++ i)
            continue;

        if ((*text)[i] == '"' && (i == 0 || (*text)[i-1] != '\\') && !in_char && in_comm == -1)
            in_string = !in_string;

        if ((*text)[i] == '\'' && (i == 0 || (*text)[i-1] != '\\') && !in_string && in_comm == -1)
            in_char = !in_char;

        if (in_string || in_char)
            continue;

        if ((*text)[i] == '<' && (*text)[i + 1] == '*')
            in_comm = i;
        if (in_comm != -1 && i >= 2 && (*text)[i - 1] == '*' && (*text)[i] == '>')
        {
            i ++;
            for (j = i; (*text)[j] != '\0'; ++ j)
                (*text)[j - (i - in_comm)] = (*text)[j];
            (*text)[j - (i - in_comm)] = '\0';
            i = in_comm;
            in_comm = -1;
        }
    }
}

bool frs_is_str_concat (char* str)
{
    char* text = malloc (strlen (str) + 1);
    strcpy (text, str);

    frs_ctrim (&text);

    bool is_string = 0;

    for (size_t i = 0; text[i] != '\0'; ++ i)
    {
        // continue if double '\'
        if (text[i] == '\\' && i >= 1 && text[i-1] == '\\' && ++ i)
            continue;

        if (text[i] == '"' && (i == 0 || text[i-1] != '\\'))
            is_string = !is_string;
        else if (!is_string)
        {
            free (text);
            return 1;
        }
    }

    free (text);

    return 0;
}

// Splits a string with a given delim saved in a *string passed as reference
// TODO: double '\\' does not work!
int frs_split (char* buffer, char delim, char*** output)
{
    int  partCount = 0; // Count of splited elements used as index for ´output´
    int  in_bracket = 0; // If ´inBracket´ bigger than 0 then ignore delim
    bool in_string  = 0; // If ´inString´ eq 1 then ignore delim & brackets
    bool in_char    = 0; // If ´inChar´ eq 1 then ignore delim & brackets

    char* ptr;
    char* lastPos;

    buffer[strlen(buffer)] = '\0'; // Set buffers string end to end of string
    (*output) = (char**) malloc (sizeof (char*)); // Allocate size

    // Loop until end of buffer is reached
    for (ptr = buffer, lastPos = buffer; *ptr != '\0'; ++ ptr)
    {
        // continue if double '\'
        if (*ptr == '\\' && (ptr != buffer && *(ptr - 1) == '\\') && ++ ptr)
            continue;

        // Check if char is string
        if (*ptr == '\"' && (ptr == buffer || *(ptr - 1) != '\\') && !in_char)
            in_string = !in_string;

        // Check if char is character
        if (*ptr == '\'' && (ptr == buffer || *(ptr - 1) != '\\') && !in_string)
            in_char = !in_char;

        if (*ptr == '(' || *ptr == '[' || *ptr == '{')
            in_bracket ++;
        if (*ptr == ')' || *ptr == ']' || *ptr == '}')
            in_bracket --;

        // Ignore delim if it is in a String/Char/Bracket
        if (*ptr != delim || in_string || in_char || in_bracket)
            continue;

        // Set splited substring in output and trim it
        (*output)[partCount] = lastPos;
        (*output)[partCount][ptr-lastPos] = '\0';
        frs_trim (&(*output)[partCount]);        

        // Count ´partCount´ up
        partCount ++;
        
        // Set ´lastPos´ which is used to generate substr
        lastPos = ptr + 1;

        // Realloc memory to string -> +1 char
        (*output) = realloc (*output, sizeof (char*) * (partCount + 1));
    }

    // Set splited substring in output and trim it
    (*output)[partCount] = lastPos;
    frs_trim (&(*output)[partCount]);

    // Return ´partCount´ used to iterate through substr list
    return partCount + 1;
}

char* frs_str_replace (char* orig, char* rep, char* with) 
{
    char* result;  // the return string
    char* ins;     // the next insert point
    char* tmp;     // varies
    int len_rep;   // length of rep (the string to remove)
    int len_with;  // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;     // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;

    len_rep = strlen(rep);

    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count

    if (!with)
        with = "";

    len_with = strlen (with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr (ins, rep); ++count)
        ins = tmp + len_rep;

    tmp = result = malloc (strlen (orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count --) 
    {
        ins = strstr (orig, rep);
        len_front = ins - orig;
        tmp = strncpy (tmp, orig, len_front) + len_front;
        tmp = strcpy  (tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}
