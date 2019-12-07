#include "interpreter.h"

#define bool char
#define true  1
#define false 0

typedef struct
{
    size_t position;
    char   constant;
    char*  function_path;
    char*  name;
} Variable;

void  fr_compiler_init ();

void  fr_compiler_run ();

int   fr_compile (const char* code, Variable** variables, const size_t pre_variable_count);
