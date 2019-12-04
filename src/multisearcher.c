// TODO: RECODE multisearcher "search" function
// TODO: Add string & bracket support


#include "multisearcher.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define CMS_CHECK(x, y) ((x & y) == y)

typedef char bool;

// Temporary variable to set ´CmsSearch´ from ´cms_add´
CmsTemplate* cms_tmp_template;

void cms_create (CmsTemplate** cms_template, void (cms_template_init) ())
{
    (*cms_template)       = cms_tmp_template = malloc (sizeof (CmsTemplate));
    (*cms_template)->size = 0;  
    (*cms_template)->list = NULL;

    // Initialize template, by calling ´cms_add´
    cms_template_init ();

    // Reset ´cms_tmp_template´ used in ´cms_add´
    cms_tmp_template = NULL;
}

void cms_add (char* syntax, CmsCallback callback, int options)
{
    if (cms_tmp_template == NULL)
        fprintf (stderr, "[CMS][ERR] → No ´cms_create´ function was called first!\n");
    else
    {
        size_t id = cms_tmp_template->size ++;

        cms_tmp_template->list = realloc (cms_tmp_template->list, sizeof (CmsSearch) * cms_tmp_template->size);

        cms_tmp_template->list[id].syntax    = syntax;
        cms_tmp_template->list[id].callback  = callback;
        cms_tmp_template->list[id].options   = options;
        cms_tmp_template->list[id].data_size = 0;

        for (size_t i = 0; i < strlen (syntax); ++ i)
            if (syntax[i] == '$' || syntax[i] == '#' || syntax[i] == '%')
                cms_tmp_template->list[id].data_size ++;
    }
}

int cms_find_next_bracket (size_t p, const char* text)
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

bool has_illigal_ascii (const char* text)
{
    for (size_t i = 0; text[i] != '\0'; ++ i)
        if (!((text[i] >= 'a' && text[i] <= 'z') || 
              (text[i] >= 'A' && text[i] <= 'Z') || 
              (text[i] >= '0' && text[i] <= '9') ||
               text[i] == '_' || text[i] == '.'))
                return 1;
    return 0;
}


char* cms_line = NULL;
char* cms_get_current_line ()
{
    return cms_line;
}

int cms_line_number = 0;
int cms_get_current_line_number ()
{
    return cms_line_number;
}

void cms_find (const char* text, CmsTemplate* cms_template)
{
    size_t text_i, text_size, text_char, text_char_i;
    size_t template_i, template_size;
    size_t i, j;

    size_t template_syntax_i;
    char   template_char;

    size_t template_data_size;
    char*  template_syntax;
    int    template_options;

    template_size = cms_template->size;
    text_size     = strlen (text);

    // Reset
    cms_line = malloc (1);
    cms_line[0] = '\0';

    // Loop through string
    for (text_i = 0; text_i < text_size; ++ text_i)
    {
        if (text_i == 0 || text[text_i] == '\n')
        {
            cms_line_number ++;

            size_t next_endl;

            for (next_endl = text_i + 1; text[next_endl] != '\n' && text[next_endl] != '\0'; ++ next_endl);

            if (next_endl == text_i + 1)
                strcpy (cms_line = malloc (1), "");
            else
            {
                size_t len = next_endl - text_i + ((text_i == 0 && text[text_i] != '\n') ? 1 : 0);
                cms_line = malloc (len);
                for (size_t i = text_i + 1; i < text_i + len; ++ i)
                    cms_line[i - text_i - 1] = text[i - ((text_i == 0 && text[text_i] != '\n') ? 1 : 0)];
                cms_line[len-1] = '\0';
            }
        }

        // Loop through template's syntaxes
        for (template_i = 0; template_i < template_size; ++ template_i)
        {
            template_options   = cms_template->list[template_i].options;
            template_data_size = cms_template->list[template_i].data_size;
            template_syntax    = cms_template->list[template_i].syntax;

            // Loop through string of template's syntax
            text_char_i = text_i;

            // Create data for template
            CmsData* data   = template_data_size ? malloc (sizeof (CmsData) * template_data_size) : NULL;
            int      data_i = 0;

            for (template_syntax_i = 0; ; ++ template_syntax_i)
            {
                // Set ´template_char´ and ´text_char´ which are going to be compared later
                template_char = template_syntax[template_syntax_i];
                text_char     = text[text_char_i];

                // if ´template_char´ is end of string, the right syntax was found in ´text´
                if (template_char == '\0')
                {
                    // Reached end of template's syntax
                    if (cms_template->list[template_i].callback != NULL) 
                        cms_template->list[template_i].callback (data, template_data_size);
                    text_i = text_char_i - 1; // skip to last point found TODO: Add option for ´MULTIPLE_MODE´
                    break;
                }

                // Exit loop, index ´text_char_i´ is already bigger than ´text_size´, no syntax was found in ´text´
                if (text_char_i >= text_size)
                    break;

                // If option ´CMS_IGNORE_UPPER_LOWER_CASE´ is the case, convert ´template_char´ and ´text_char´ to lower case
                if (CMS_CHECK (template_options, CMS_IGNORE_UPPER_LOWER_CASE))
                {
                    if (template_char >= 'A' && template_char <= 'Z')
                        template_char -= 'A' - 'a';
                    if (text_char >= 'A' && text_char <= 'Z')
                        text_char -= 'A' - 'a';
                }

                if (template_char == '$' || template_char == '#' || template_char == '%')
                {
                    int new_text_char_i = text_char_i;

                    // next ´template_syntax´ with > ' '
                    for (i = template_syntax_i + 1; template_syntax[i] <= ' '; ++ i);

                    // Loop ends at Space, Endl, Tab, ... and at the end of ´test´ => '\0'
                    if (template_char == '$')
                    {
                        while (text[new_text_char_i] > ' ' && text[new_text_char_i] != template_syntax[i])
                            new_text_char_i ++;
                    }
                    else if (template_char == '#')
                    {
                        while (text[new_text_char_i] > ' ' && text[new_text_char_i] != template_syntax[i])
                            new_text_char_i ++;
                    }
                    else if (template_char == '%')
                    {
                        // last ´template_syntax´ with > ' '
                        for (new_text_char_i -= 1; text[new_text_char_i] <= ' '; -- new_text_char_i);
                       /* if ((text[new_text_char_i] == '('  || text[new_text_char_i] == '['  || text[new_text_char_i] == '{') && CMS_CHECK (template_options, CMS_USE_BRACKET_SEARCH_ALGORITHM))
                            new_text_char_i = cms_find_next_bracket (new_text_char_i, text);
                        else while (text[new_text_char_i] != template_syntax[i])
                            new_text_char_i ++;
                    */
                        // might be wrong!
                        while (text[new_text_char_i] != template_syntax[i])
                            if ((text[new_text_char_i] == '('  || text[new_text_char_i] == '['  || text[new_text_char_i] == '{') 
                                    && CMS_CHECK (template_options, CMS_USE_BRACKET_SEARCH_ALGORITHM))
                                new_text_char_i = cms_find_next_bracket (new_text_char_i, text);
                            else
                                new_text_char_i ++;

                    }

                    // Size of new ´data_str´
                    int  size = new_text_char_i - text_char_i;
                    char data_str[size];

                    // Copy data to ´data_str´
                    for (i = 0; i < size; ++ i)
                        data_str[i] = text[text_char_i + i];
                    data_str[i] = '\0'; // end character

                    // Check if '#' has legall ascii charactors 
                    if (template_char == '#' && (data_str[0] == '\0' || has_illigal_ascii (data_str)))
                        break; 

                    // Update ´text_char_i´ to keep on comparing the ´text´ with the given syntax
                    text_char_i = new_text_char_i;

                    // If ´CMS_IGNORE_SPACING_LENGTH´ is activated, then the next char must be a spacing
                    if ( CMS_CHECK (template_options, CMS_IGNORE_SPACING_LENGTH) &&
                        !CMS_CHECK (template_options, CMS_IGNORE_SPACING)        &&
                         template_syntax[template_syntax_i + 1] <= ' '           &&
                         template_char == '%')
                            text_char_i --;

                    // Copy ´data_str´ to data array
                    data[data_i] = malloc (strlen (data_str) + 1);
                    data_str[strlen(data_str)] = '\0'; // end character
                    strcpy (data[data_i], data_str);
                    data_i ++;
                    continue;
                }

                if (CMS_CHECK (template_options, CMS_IGNORE_SPACING_LENGTH) && text[text_char_i] == ' ' && template_char == ' ')
                {
                    while (text[text_char_i] <= ' ' && text[text_char_i] != template_syntax[template_syntax_i + 1])
                        text_char_i ++;
                    continue;
                }
                else if (CMS_CHECK (template_options, CMS_IGNORE_SPACING) && template_char == ' ')
                {
                    while (text[text_char_i] <= ' ' && text[text_char_i] != template_syntax[template_syntax_i + 1])
                        text_char_i ++;
                    continue;
                }
                // If ´template_char´ is not equal ´text_char´ than the ´text´ is not equal to the given syntax
                if (template_char != text_char)
                    break;

                // Update text index
                text_char_i ++;
            }

            if (data) free (data);
        }
    }

    free (cms_template->list);
    free (cms_template);
    free (cms_line);
}
