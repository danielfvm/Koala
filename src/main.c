#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define KOALA_VERSION "0.3-a1"

#include "multisearcher.h"
#include "interpreter.h"
#include "lexer.h"
#include "library.h"
#include "../lib/std/stdlib.h"

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


/* Koala's main function < Program starts here */
int main (int argc, char** argv)
{
    Variable* variables;
    size_t variables_count;

    variables = malloc (sizeof (Variable));
    variables_count = 0;

    // Missing Koala-Filepath -> Error
    if (argc < 2)
    {
        fprintf (stderr, "Missing File argument!\n");
        return EXIT_SUCCESS;
    }

    // Add STD functions
    kl_lib_std_init ();

    // Initialize the compiler
    kl_lex_compiler_init ();

    // Create Value* for argv
    Value* _argv_ = malloc (sizeof (Value) * argc);
    for (size_t i = 0; i < argc; ++ i)
        _argv_[i] = VALUE_STR (argv[i]);

    // Add Variables with System Information & Universal-Constants
    kl_lex_add_variable (&variables, &variables_count, "local", "_OS_",   true, VALUE_STR   (OS));
    kl_lex_add_variable (&variables, &variables_count, "local", "_PI_",   true, VALUE_FLOAT (3.1415926535));
    kl_lex_add_variable (&variables, &variables_count, "local", "_E_",    true, VALUE_FLOAT (2.7182818284));
    kl_lex_add_variable (&variables, &variables_count, "local", "_argc_", true, VALUE_INT   (argc));
    kl_lex_add_variable (&variables, &variables_count, "local", "_argv_", true, VALUE_LIST  (
        _argv_,
        argc
    ));

    // Translate to Koala-Register format
    kl_lex_compile (kl_util_read_file (argv[1]), &variables, &variables_count, true);

    // Execute Koala's registers
    kl_lex_compiler_run ();

    // Free Value*
    free (_argv_);

    return EXIT_SUCCESS;
}
