#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "multisearcher.h"
#include "interpreter.h"
#include "gnumber.h"
#include "compiler.h"

char* read_file (const char* filepath)
{
    char* buffer;
    FILE* file;
    size_t len;
    
    if ((file = fopen (filepath, "r")) == NULL)
    {
        fprintf (stderr, "Failed to open file ´%s´\n", filepath);
        exit (EXIT_FAILURE);
    }

    buffer = NULL;

    if (getdelim (&buffer, &len, '\0', file) == -1)
    {
        fprintf (stderr, "Failed to read file ´%s´\n", filepath);
        exit (EXIT_FAILURE);
    }

    return buffer;
}

/*
void* cast_to_void_ptr (float value)
{
    float* ptr = malloc (sizeof (float));
    (*ptr) = value;
    return ptr;
}
*/

int main (int argc, char** argv)
{
    fr_compiler_init ();

    Variable* variables;

    if (!(variables = malloc (sizeof (Variable))))
    {
        fprintf (stderr, "Failed to create list ´variables´ (Main)\n");
        return EXIT_FAILURE;
    }

    fr_compile (read_file ("examples/example.frs"), &variables, 0);

    fr_compiler_run ();

    return EXIT_SUCCESS;
}
