#define UNKNOWN -1

#include <stddef.h>

#include "interpreter.h"

typedef Value (kl_lib_func)(int argc, Value* argv);

typedef struct 
{
    kl_lib_func* func;
    char*  name;
    size_t id;
} kl_lib_function;


void kl_lib_add_function (const char* name, kl_lib_func* func);

int kl_lib_get_function_by_name (const char* name);

Value kl_lib_exec_function (const int id, int argc, Value* argv);

void* _kl_lib_get_value (Value value);

#define NUMBER(value) ( (value).data_type == DT_FLOAT ? *(float*)_kl_lib_get_value(value) : (intptr_t)_kl_lib_get_value(value) )
