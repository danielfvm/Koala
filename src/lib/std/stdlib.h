#include <inttypes.h>
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

// TODO: change read Function to return state & changes string as ref
Value kl_lib_std_readFile (int argc, Value* argv) {
    if (argc <= 0 || argv[0].data_type != DT_STRING)
        return VALUE_INT (0);
    return VALUE_STR (kl_util_read_file (argv[0].value));
}

void kl_lib_std_init () {
    kl_lib_add_function ("getVersion",  kl_lib_std_getVersion);
    kl_lib_add_function ("len",         kl_lib_std_len);
    kl_lib_add_function ("exit",        kl_lib_std_exit);
    kl_lib_add_function ("abs",         kl_lib_std_abs);
    kl_lib_add_function ("sin",         kl_lib_std_sin);
    kl_lib_add_function ("cos",         kl_lib_std_cos);
    kl_lib_add_function ("tan",         kl_lib_std_tan);
    kl_lib_add_function ("readFile",    kl_lib_std_readFile);
}
