#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "multisearcher.h"
#include "interpreter.h"
#include "gnumber.h"
#include "compiler.h"

char* read_file (const char* filepath)
{
    FILE* file;
    char* buffer = 0;
    int length;

    if ((file = fopen (filepath, "r")) == NULL)
        return NULL;

    fseek (file, 0, SEEK_END);
    length = ftell (file);
    fseek (file, 0, SEEK_SET);
    buffer = malloc (length);

    if (!buffer)
    {
        fprintf (stderr, "Failed loading buffer in ´read_file´\n");
        return NULL;
    }

    fread (buffer, 1, length, file);

    fclose (file);

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

    char* code = read_file ("examples/test.frs");

    fr_compile (code, variables, 0);

    fr_compiler_run ();

    return EXIT_SUCCESS;
}
