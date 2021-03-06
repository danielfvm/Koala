#include "util.h"
#include "multisearcher.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>


void error (const char* msg, ...)
{
    int line_number = cms_get_current_line_number ();
    int nDigits = floor (log10 (abs (line_number))) + 1;

    if (line_number <= 0)
    {
        printf ("Line Number cannot be 0 or smaller!\n");
        exit (EXIT_FAILURE);
    }

    char* arrow = malloc (nDigits * 3 + 1);
    int i;

    *arrow = '\0';
    for (i = 0; i < nDigits; ++ i)
        strcat (arrow, "─");
    arrow[i * 3] = '\0';

    char* line = cms_get_current_line ();
    kl_util_trim (&line);

    va_list arg;
    va_start (arg, msg);

    fprintf (stderr, "\x1b[90m[\x1b[33m%d\x1b[90m]\x1b[92m─►\x1b[93m %s\n \x1b[92m└%s─► \x1b[91m(ERR) ", line_number, line, arrow);
    vfprintf (stderr, msg, arg);
    fprintf (stderr, "\x1b[0m\n");

    va_end (arg);
    free (arrow);

    exit (EXIT_SUCCESS);
}

void warning (const char* msg, const void* variablen, ...)
{
    int line_number = cms_get_current_line_number ();
    int nDigits = floor (log10 (abs (line_number))) + 1;

    char* arrow = malloc (nDigits * 3 + 1);
    int i;

    *arrow = '\0';
    for (i = 0; i < nDigits; ++ i)
        strcat (arrow, "─");
    arrow[i * 3 + 1] = '\0';

    char* line = cms_get_current_line ();
    kl_util_trim (&line);
    fprintf (stderr, "\x1b[90m[\x1b[33m%d\x1b[90m]\x1b[92m─►\x1b[93m %s\n \x1b[92m└%s─► \x1b[95m(WARN) ", line_number, line, arrow);
    fprintf (stderr, msg, variablen);
    fprintf (stderr, "\x1b[0m\n");

    free (arrow);
}

char* kl_util_read_file (const char* filepath)
{
    FILE*  file;
    char*  buffer;
    size_t size;

    if (!(file = fopen (filepath, "r"))) 
        fprintf (stderr, "Failed to open file ´%s´\n", filepath), exit (EXIT_FAILURE);

    fseek (file , 0L, SEEK_END);
    size = ftell (file);
    rewind (file);

    /* allocate memory for entire content */
    if (!(buffer = calloc (1, size + 1)))
        fprintf (stderr, "Memory alloc failed in ´kl_util_read_file´\n"), exit (EXIT_FAILURE);

    /* copy the file into the buffer */
    if(!fread (buffer, size, 1, file))
        fclose(file), free (buffer), fprintf (stderr, "Reading failed in ´kl_util_read_file´\n"), exit (EXIT_FAILURE);

    fclose (file);

    return buffer;
}

void kl_util_trim_front_end (char** str)
{
    size_t size = strlen (*str);

    if (size <= 1)
        return;

    (*str)[size - 1] = '\0';
    (*str) ++;
}

bool kl_util_has_illigal_ascii (const char* text)
{
    for (size_t i = 0; text[i] != '\0'; ++ i)
        if (!((text[i] >= 'a' && text[i] <= 'z') || 
              (text[i] >= 'A' && text[i] <= 'Z') || 
              (text[i] >= '0' && text[i] <= '9') ||
               text[i] == '_' || text[i] == '.' || text[i] == ':'))
                return true;
    return false;
}

int kl_util_find_string_end (size_t p, const char* text)
{
    size_t string_len;
    char   string_end;
    int    old_p = p + 1;

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

int kl_util_find_next_bracket (size_t p, const char* text)
{
    size_t inside_brackets  = 0;
    size_t text_size        = strlen (text);

    if (p >= text_size)
    {
        fprintf (stderr, "[CMS][ERR] → first char out of text size\n");
        return -1;
    }

    char bracket_close;
    char bracket_open = text[p];

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
        if (text[p] == '\\' && (p >= 1 && text[p - 1] != '\\') && ++ p)
            continue;

        if (text[p] == '\"' && (p <= 0 || text[p - 1] != '\\') && !in_char)
            in_string = !in_string;
        if (text[p] == '\'' && (p <= 0 || text[p - 1] != '\\') && !in_string)
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
    return NOT_FOUND;
}

char* kl_util_substr (const char *src, int m, int n)
{
    unsigned int len = n - m; // length of string
    int i;

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
void kl_util_trim (char** text)
{
    if (text == NULL || (*text) == NULL || **text == '\0')
        return;

    // Trim begin
    while (**text <= ' ')
        (*text) ++;

    // Trim end
    for (size_t len = strlen (*text) - 1; (*text)[len] <= ' '; -- len)
        (*text)[len] = '\0';
}

void kl_util_ctrim (char** text)
{
    if (text == NULL || (*text) == NULL)
        return;

    size_t i, j;
    bool in_string = false;
    bool in_char   = false;

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

bool kl_util_is_bracket (char c)
{
    return c == '(' || c == '[' || c == '{' || c == ')' || c == ']' || c == '}';
}

size_t kl_util_contains (char* text, char c)
{
    if (text == NULL)
        return false;

    bool in_string = false;
    bool in_char   = false;
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

        if (text[i] == c && (kl_util_is_bracket (c) || (!in_string && !in_char && !in_bracket)))
            return true + i;
    }
    return false;
}

void kl_util_filter_comment (char** text)
{
    if (text == NULL || (*text) == NULL)
        return;

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

        // Comment1: **
        if (in_comm == -1 && (*text)[i] == '*' && (*text)[i + 1] == '*')
            while ((*text)[i] != '\n' && (*text)[i] != '\0')
                for (j = i; (*text)[j] != '\n' && (*text)[j] != '\0'; ++ j)
                    (*text)[j] = (*text)[j + 1];

        // Comment2: <* *>
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

bool kl_util_is_str_concat (char* str)
{
    if (str == NULL || *str == '\0')
        return false;

    char* text = malloc (strlen (str) + 1);
    strcpy (text, str);

    kl_util_ctrim (&text);

    bool is_string = false;

    for (size_t i = 0; text[i] != '\0'; ++ i)
    {
        // continue if double '\'
        if (text[i] == '\\' && i >= 1 && text[i-1] == '\\' && ++ i)
            continue;

        if (text[i] == '$' && text[i + 1] == '{')
            i = kl_util_find_next_bracket (i + 1, text);

        if (text[i] == '"' && (i == 0 || text[i-1] != '\\'))
            is_string = !is_string;
        else if (!is_string)
        {
            free (text);
            text = NULL;
            return true;
        }
    }

    free (text);
    text = NULL;

    return false;
}

// Splits a string with a given delim saved in a *string passed as reference
// TODO: double '\\' does not work!
int kl_util_split (char* buffer, char delim, char*** output)
{
    if (buffer == NULL || *buffer == '\0')
        return 0;

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
        kl_util_trim (&(*output)[partCount]);        

        // Count ´partCount´ up
        partCount ++;
        
        // Set ´lastPos´ which is used to generate substr
        lastPos = ptr + 1;

        // Realloc memory to string -> +1 char
        (*output) = realloc (*output, sizeof (char*) * (partCount + 1));
    }

    // Set splited substring in output and trim it
    (*output)[partCount] = lastPos;
//    (*output)[partCount][ptr-lastPos] = '\0';
    kl_util_trim (&(*output)[partCount]);

    // Return ´partCount´ used to iterate through substr list
    return partCount + 1;
}

char* kl_util_str_replace (char* orig, char* rep, char* with) 
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
    for (count = 0; (tmp = strstr (ins, rep)); ++count)
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
