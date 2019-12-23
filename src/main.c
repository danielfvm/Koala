#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "multisearcher.h"
#include "interpreter.h"
#include "gnumber.h"
#include "compiler.h"

int main (int argc, char** argv)
{
    fr_compiler_init ();

    Variable* variables;
    size_t    variables_count = 0;

    if (!(variables = malloc (sizeof (Variable))))
    {
        fprintf (stderr, "Failed to create list ´variables´ (Main)\n");
        return EXIT_FAILURE;
    }

    if (argc < 2)
    {
        fprintf (stderr, "Missing File argument!\n");
        return EXIT_FAILURE;
    }

    fr_compile (frs_read_file (argv[1]), &variables, &variables_count, true);

    fr_compiler_run ();

    return EXIT_SUCCESS;
}
