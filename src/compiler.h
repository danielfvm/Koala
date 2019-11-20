#include "interpreter.h"

typedef struct
{
    size_t position;
    char*  name;
} Variable;

void  fr_compiler_init ();

void  fr_compiler_run ();

int   fr_compile (const char* code, Variable* variables, const size_t pre_variable_count);
