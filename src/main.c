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
    ssize_t bytes_read = getdelim( &buffer, &len, '\0', file);

    if (bytes_read == -1)
    {
        fprintf (stderr, "Failed to read file ´%s´\n", filepath);
        exit (EXIT_FAILURE);
    }

    return buffer;
}

int main (int argc, char** argv)
{
    fr_compiler_init ();

    Variable* variables = malloc (sizeof (Variable));

    if (!variables)
    {
        fprintf (stderr, "Failed to create list ´variables´ (Main)\n");
        return EXIT_FAILURE;
    }

    char* code = read_file ("examples/example.frs");

    fr_compile (code, variables, 0);

    fr_compiler_run ();

    return EXIT_SUCCESS;
}
