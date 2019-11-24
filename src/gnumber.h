// gnumber -> GNU  +  Number
// gba     -> Gnumber Bool   Algebra
// gna     -> Gnumber Normal Algebra

#define bool char

#include "interpreter.h"

char* _substr (const char* str, int m, int n);

/** Simple calculation with basic computes **/
bool  gba_calculation_simple (const char* calc);

/** Calculation with basic computes and brackets **/
bool  gba_calculation (const char* calc);

/** Simple calculation with basic computes **/
float gna_calculation_simple (const char* calc);

/** Simple calculation with basic computes which are converted to registry commands **/
Value gna_registry_calculation_simple (Registry** register_list, const char* calc, Value (*fr_convert_to_value)(char*), size_t (*var_add)(char*));

/** Calculation with basic computes and brackets **/
float gna_calculation (const char* calc);
