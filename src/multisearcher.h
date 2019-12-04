#include <stddef.h>
// #define CMS_CALLBACK(x) ({ void _(CmsData* data, int size) x _; })
#define CMS_LIST(x)     ({ void _() x; _; })

typedef void (*CmsData);
typedef void (*CmsCallback) (CmsData* data, int size);


enum CmsOption
{
    CMS_NONE,                           // None option selected
    CMS_IGNORE_UPPER_LOWER_CASE,        // Ignores if char is upper or down case
    CMS_IGNORE_SPACING_LENGTH,          // Ignores if there are multiple spaces instead of the given spaces in template
    CMS_IGNORE_SPACING,                 // Ignores if there are spaces between text
    CMS_USE_BRACKET_SEARCH_ALGORITHM,   // Ignores if there are spaces between text
    CMS_IGNORE_STRING
};


typedef struct
{
    CmsCallback callback;
    int         data_size;
    int         options;
    char*       syntax;
} CmsSearch;

typedef struct
{
    unsigned int size;
    CmsSearch*   list;
} CmsTemplate;

char* cms_get_current_line ();

int cms_get_current_line_number ();

int cms_find_next_bracket (size_t p, const char* text);

/*
 *   Here you can create your own template, which is used in ´cms_find´ as manuel
 *   to find certain syntax in a text.
 *
 *   1. ´cms_template´ Make a reference to your created struct.
 *
 *   2. ´cms_template_init´ You will need to call ´cms_add´ from ´CMS_LIST´.
 *                          -> CMS_LIST ({ cms_add (...); });
 *   USAGE:
 *      cms_create (&template, size, CMS_LIST ( ... ));
 */
void cms_create (CmsTemplate** cms_template, void (cms_template_init) ());

/*
 *   Here you are adding the ´CmsSearch´ struct to the array of the template
 *   by calling this function from inside of ´CMS_LIST´ (similar to lambda).
 *   
 *   1. ´syntax´ Here you define how the syntax should look like. Special
 *               Characters are by default:
 *                  - '$' Used for single word
 *                  - '%' Used for multiple words and symbols
 *
 *   2. ´callback´ Is the function which will be called when syntax was found.
 *
 *   3. ´options´ There are multiple options you can choose from, you can use
 *                more than 1 by spliting those options with a ´|´.
 *
 */
void cms_add (char* syntax, CmsCallback callback, int options);

/*
 *   With this function you actually search after the syntax.
 *
 *   1. ´text´ Your syntax will be searched in this cstring.
 *
 *   2. ´cms_template´ Your template created with ´cms_create´.
 */
void cms_find (const char* text, CmsTemplate* cms_template);
