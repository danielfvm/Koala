// TODO: RECODE multisearcher "search" function
// TODO: Add string & bracket support


#include "multisearcher.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define CMS_CHECK(x, y) ((x & y) == y)

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
            if (syntax[i] == '$')
                cms_tmp_template->list[id].data_size ++;
    }
}

int _cms_find_next_bracket (size_t p, const char* text)
{
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
            fprintf (stderr, "[CMS][ERR] → Wrong first char used in ´_cms_find_next_bracket´. First char musst be '(', '[' or '{'!\n");
            return -1;
    }

    int in_string = 0;
    int in_char   = 0;

    // Searching after closing bracket
    for (; p < text_size; ++ p)
    {
        if (text[p] == '\"' && (p <= 0 || text[p-1] != '\\'))
            in_string = !in_string;
        if (text[p] == '\'' && (p <= 0 || text[p-1] != '\\'))
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

    // Loop through string
    for (text_i = 0; text_i < text_size; ++ text_i)
    {
        // Loop through template's syntaxes
        for (template_i = 0; template_i < template_size; ++ template_i)
        {
            template_options   = cms_template->list[template_i].options;
            template_data_size = cms_template->list[template_i].data_size;
            template_syntax    = cms_template->list[template_i].syntax;

            // Loop through string of template's syntax
            text_char_i = text_i;

            // Create data for template
            CmsData* data   = malloc (sizeof (CmsData) * template_data_size);
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

                if (template_char == '$' || template_char == '%')
                {
                    int new_text_char_i = text_char_i;

                    // next ´template_syntax´ with > ' '
                    for (i = template_syntax_i + 1; template_syntax[i] <= ' '; ++ i);

                    // Loop ends at Space, Endl, Tab, ... and at the end of ´test´ => '\0'
                    if (template_char == '$')
                    {
                        //if ((template_options & CMS_IGNORE_SPACING) == CMS_IGNORE_SPACING && text_char <= ' ')
                        //    while (text[new_text_char_i] <= ' ')
                        //        new_text_char_i ++;

                        // TODO: Check if this is a reliable alternative to the uncomment code
                        // ----> Add ´CMS_IGNORE_SPACING´ & ´CMS_IGNORE_SPACING_LENGTH´ option to it
                        while (text[new_text_char_i] > ' ' && text[new_text_char_i] != template_syntax[i])
                            new_text_char_i ++;

                        /*if ((template_options & CMS_IGNORE_SPACING) == CMS_IGNORE_SPACING)
                        {
                            char found_space = 0;
                            while ((text[new_text_char_i] != template_syntax[i]))
                            {
                                if (text[new_text_char_i] <= ' ')
                                    found_space = 1;
                                else if (found_space == 1)
                                    break;
                                new_text_char_i ++;
                            }
                        }
                        else while (text[new_text_char_i] > ' ' && (text[new_text_char_i] != template_syntax[i]))
                            new_text_char_i ++;*/
                    }
                    else if (template_char == '%')
                    {
                        // last ´template_syntax´ with > ' '
                        for (new_text_char_i -= 1; text[new_text_char_i] <= ' '; -- new_text_char_i);
// might be wrong!
                        if ((text[new_text_char_i] == '('  || text[new_text_char_i] == '['  || text[new_text_char_i] == '{') && CMS_CHECK (template_options, CMS_USE_BRACKET_SEARCH_ALGORITHM))
                            new_text_char_i = _cms_find_next_bracket (new_text_char_i, text);
                        else while (text[new_text_char_i] != template_syntax[i])
                            new_text_char_i ++;
                    }

                    // Size of new ´data_str´
                    int  size = new_text_char_i - text_char_i;
                    char data_str[size];

                    // Copy data to ´data_str´
                    for (i = 0; i < size; ++ i)
                        data_str[i] = text[text_char_i + i];
                    data_str[i] = '\0'; // end character

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

            free (data);
        }
    }

    free (cms_template->list);
    free (cms_template);
}
