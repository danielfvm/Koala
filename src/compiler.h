#include "interpreter.h"
#include "util.h"

typedef struct
{
    size_t position;
    char   constant;
    char*  function_path;
    char*  name;
} Variable;

void error (const char* msg, const void* variablen, ...);

void  fr_compiler_init ();

void  fr_add_variable (Variable** variables, size_t* variable_count, const char* path, const char* name, const bool constant, Value value);

void  fr_compiler_run ();

int   fr_compile (char* code, Variable** variables, size_t* pre_variable_count, const bool reset_variables);
