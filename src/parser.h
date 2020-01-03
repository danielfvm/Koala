#include "interpreter.h"

typedef struct
{
    size_t position;
    char   constant;
    char*  function_path;
    char*  name;
} Variable;

void  kl_parse_compiler_init ();

void  kl_parse_add_variable (Variable** variables, size_t* variable_count, const char* path, const char* name, const bool constant, Value value);

void  kl_parse_compiler_run ();

int   kl_parse_compile (char* code, Variable** variables, size_t* pre_variable_count, const bool reset_variables);
