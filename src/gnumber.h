// gnumber -> GNU  +  Number

#include "interpreter.h"

/** Calculation with basic computes ´+/-*´ which are converted to registry commands **/
Value gna_registry_calculation_simple (Registry** register_list, const char* calc, Value (*fr_convert_to_value)(char*));

/** Calculation with AND `&` and OR `|` which are converted to registry commands **/
Value gna_registry_boolean_algebra (Registry** register_list, const char* calc, Value (*fr_convert_to_value)(char*));
