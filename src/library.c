#include "library.h"

#include <stdlib.h>
#include <string.h>

kl_lib_function*  kl_lib_function_list = NULL;
size_t            kl_lib_function_size = 0;

void kl_lib_add_function (const char* name, kl_lib_func* func)
{
    if (kl_lib_function_list != NULL)
        kl_lib_function_list = realloc (kl_lib_function_list, sizeof (kl_lib_function) * (kl_lib_function_size + 1));
    else
        kl_lib_function_list = malloc (sizeof (kl_lib_function));
    
    strcpy (kl_lib_function_list[kl_lib_function_size].name = malloc (strlen (name) + 1), name);
    kl_lib_function_list[kl_lib_function_size].id = kl_lib_function_size;
    kl_lib_function_list[kl_lib_function_size].func = func;

    kl_lib_function_size ++; 
}

int kl_lib_get_function_by_name (const char* name)
{
    for (size_t i = 0; i < kl_lib_function_size; ++ i)
        if (strcmp (kl_lib_function_list[i].name, name) == 0)
            return kl_lib_function_list[i].id;
    return UNKNOWN;
}

Value kl_lib_exec_function (const int id, int argc, Value* argv)
{
    return kl_lib_function_list[id].func (argc, argv); 
}

void* _kl_lib_get_value (Value value) 
{
    return value.value;
}
