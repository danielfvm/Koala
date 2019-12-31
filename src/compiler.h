#include "interpreter.h"
#include "util.h"

#include <stdarg.h>

typedef struct
{
    size_t position;
    char   constant;
    char*  function_path;
    char*  name;
} Variable;

void error (const char* msg, ...);

void  kl_lex_compiler_init ();

void  kl_lex_add_variable (Variable** variables, size_t* variable_count, const char* path, const char* name, const bool constant, Value value);

void  kl_lex_compiler_run ();

int   kl_lex_compile (char* code, Variable** variables, size_t* pre_variable_count, const bool reset_variables);
