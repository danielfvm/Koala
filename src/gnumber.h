// gnumber -> GNU  +  Number

#include "interpreter.h"

/** Simple calculation with basic computes which are converted to registry commands **/
Value gna_registry_calculation_simple (Registry** register_list, const char* calc, Value (*fr_convert_to_value)(char*));
