#include "interpreter.h"
#include "util.h"

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
