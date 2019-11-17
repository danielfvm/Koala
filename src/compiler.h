#include "interpreter.h"

typedef struct
{
    size_t position;
    char*  name;
} Variable;

int   var_get_pos_by_name (const char* name);

void  var_add (char* name, size_t position);

Value fr_convert_to_value (char* text);

void  fr_compiler_init ();

void  fr_compiler_run ();

int   fr_compile (const char* code);
