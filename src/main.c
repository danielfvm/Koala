#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "multisearcher.h"
#include "interpreter.h"
#include "gnumber.h"
#include "compiler.h"

#ifdef _WIN32
#define OS "Windows 32"
#elif _WIN64
#define OS "Windows 64"
#elif __APPLE__ || __MACH__
#define OS "OSX"
#elif __linux__
#define OS "Linux"
#elif __FreeBSD__
#define OS "FreeBSD"
#elif __unix || __unix__
#define OS "Unix"
#else
#define OS "Unknown"
#endif


int main (int argc, char** argv)
{
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

    fr_compiler_init ();

    fr_add_variable (&variables, &variables_count, "local", "__os__", true, VALUE_STR (OS));

    fr_compile (frs_read_file (argv[1]), &variables, &variables_count, true);

    fr_compiler_run ();

    return EXIT_SUCCESS;
}
