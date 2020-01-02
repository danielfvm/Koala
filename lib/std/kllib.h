#include "../../src/interpreter.h"

#include <inttypes.h>
#include <string.h>
#include <math.h>

Value kl_lib_std_getVersion (int argc, Value* argv) {
    return VALUE_STR (KOALA_VERSION);
}

Value kl_lib_std_len (int argc, Value* argv) {
    if (argc <= 0)
        return VALUE_INT (0);
    if (argv[0].data_type == DT_STRING)
        return VALUE_INT (strlen (argv[0].value));
    if (argv[0].data_type == DT_LIST)
        return VALUE_INT (argv[0].size);
    return VALUE_INT (0);
}

Value kl_lib_std_exit (int argc, Value* argv) {
    exit (argc <= 0 ? 0 : NUMBER (argv[0]));
}

Value kl_lib_std_abs (int argc, Value* argv) {
    if (argc <= 0)
        return VALUE_INT (0);

    if (argv[0].data_type == DT_FLOAT)
        return VALUE_FLOAT (NUMBER (argv[0]) < 0 ? NUMBER (argv[0]) * (-1) : NUMBER (argv[0]));
    if (argv[0].data_type == DT_INT || argv[0].data_type == DT_CHAR)
        return VALUE_INT   (NUMBER (argv[0]) < 0 ? NUMBER (argv[0]) * (-1) : NUMBER (argv[0]));

    return VALUE_INT (0);
}

Value kl_lib_std_sin (int argc, Value* argv) {
    return VALUE_FLOAT (sin (argc <= 0 ? 0 : NUMBER (argv[0])));
}

Value kl_lib_std_cos (int argc, Value* argv) {
    return VALUE_FLOAT (cos (argc <= 0 ? 0 : NUMBER (argv[0])));
}

Value kl_lib_std_tan (int argc, Value* argv) {
    return VALUE_FLOAT (tan (argc <= 0 ? 0 : NUMBER (argv[0])));
}

Value kl_lib_std_sqrt (int argc, Value* argv) {
    return VALUE_FLOAT (sqrt (argc <= 0 ? 0 : NUMBER (argv[0])));
}

Value kl_lib_std_list (int argc, Value* argv) {
    int size = argc <= 0 ? 0 : NUMBER (argv[0]);
    Value* values = malloc (sizeof (Value) * size);
    for (int i = 0; i < size; ++ i)
        values[i] = VALUE_INT (0);
    return VALUE_LIST (values, size);
}

// TODO: Support float!
Value kl_lib_std_range (int argc, Value* argv) {
    int from = argc <= 0 ? 0 : NUMBER (argv[0]);
    int to   = argc <= 1 ? 0 : NUMBER (argv[1]);
    int step = argc <= 2 ? 1 : NUMBER (argv[2]);
    int size = abs (from - to) / step + 1;

    Value* values = malloc (sizeof (Value) * size);

    int i;

    if (from < to) for (i = 0; i < size; ++ i)
        values[i] = VALUE_INT (i * step + from);
    else if (from > to) for (i = 0; i < size; ++ i)
        values[i] = VALUE_INT (from - i * step);

    return VALUE_LIST (values, size);
}

Value kl_lib_std_readFile (int argc, Value* argv) {
    if (argc <= 1 || argv[0].data_type != DT_STRING || argv[1].data_type != DT_INT)
        return VALUE_INT (false);

    FILE*  file;
    char*  buffer;
    size_t size;

    if (!(file = fopen (argv[0].value, "r"))) 
        return VALUE_INT (false);

    fseek (file , 0L, SEEK_END);
    size = ftell (file);
    rewind (file);

    /* allocate memory for entire content */
    if (!(buffer = calloc (1, size + 1)))
        return VALUE_INT (false);

    /* copy the file into the buffer */
    if(!fread (buffer, size, 1, file))
        return VALUE_INT (false);

    kl_intp_set_pointer (NUMBER (argv[1]), VALUE_STR (buffer));

    free (buffer);

    fclose (file);

    return VALUE_INT (true);
}

void kl_lib_std_init () {
    kl_lib_add_function ("getVersion",  kl_lib_std_getVersion);
    kl_lib_add_function ("len",         kl_lib_std_len);
    kl_lib_add_function ("exit",        kl_lib_std_exit);
    kl_lib_add_function ("abs",         kl_lib_std_abs);
    kl_lib_add_function ("sin",         kl_lib_std_sin);
    kl_lib_add_function ("cos",         kl_lib_std_cos);
    kl_lib_add_function ("tan",         kl_lib_std_tan);
    kl_lib_add_function ("sqrt",        kl_lib_std_sqrt);
    kl_lib_add_function ("readFile",    kl_lib_std_readFile);
    kl_lib_add_function ("list",       kl_lib_std_list);
    kl_lib_add_function ("range",      kl_lib_std_range);
}
