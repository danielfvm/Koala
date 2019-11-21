#include "gnumber.h"
#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* _substr (const char *src, int m, int n)
{
    unsigned int len = n - m; // length of string
    unsigned int i;

    // allocate (len + 1) chars for destination (+1 for extra null character)
    char *dest = malloc (sizeof (char) * (len + 1));

    // extracts characters between m'th and n'th index from source string
    // and copy them into the destination string
    for (i = m; i < n && (*src != '\0'); ++ i)
    {
        *dest = *(src + i);
        dest ++;
    }

    *dest = '\0';

    return dest - len; // destination string
}

void _trim (char** text)
{
    // Trim begin
    while (**text <= ' ')
        (*text) ++;

    // Trim end
    for (unsigned int len = strlen (*text) - 1; (*text)[len] <= ' '; -- len)
        (*text)[len] = '\0';
}


int _find_close_bracket (const char* str, const char open, const char close, int p)
{
    for (int brackets = 0; str[p] != '\0'; ++ p)
    {
        if (str[p] == open)
            brackets ++;
        else if (str[p] == close && (-- brackets) <= 0)
            return p;
    }

    return -1;
}

int _find_brackets (const char* str)
{
    for (unsigned int i = 0; str[i] != '\0'; ++ i)
        if (str[i] == '(')
            return 1;
    return 0;
}



bool* _gba_conv_calculation (const char* calc, unsigned int* size)
{
    unsigned int calc_len = strlen (calc);
    unsigned int array_i  = 0;
    unsigned int last_i   = 0;
    unsigned int i, j;

    // Find maximum element count by searching after computes 
    for (i = 0, *size = 1; i < strlen (calc); ++ i)
        if (calc[i] == '&' || calc[i] == '|')
            *size += 2;

    // Allocate memory to array containing values
    bool* array = malloc (sizeof (bool) * (*size));

    for (i = 0; i < calc_len; ++ i)
    {
        // Skips at beginning & if it is not a compute 
        if ((calc[i] != '&' && calc[i] != '|') || i == 0)
            continue;

        // Insert value to bool array
        array[array_i ++] = atof (_substr (calc, last_i, i));

        // Insert compute to bool array
        array[array_i ++] = calc[i];
        last_i = i + 1;

        // Moves index till no compute was found, or any value was found
        // TODO: Seems todo nothing ... find out what this line effect in finish result
        for (j = 1; (i + j < calc_len) && (calc[i + j] == '&' || calc[i + j] == '|'); ++ j, ++ i);
    }

    // Insert value
    array[array_i ++] = atof (_substr (calc, last_i, i));

    return array;
}

bool gba_calculation_simple (const char* calc)
{
    unsigned int i, j, size;
    float* array;

    _gba_conv_calculation (calc, &size);

    /** Starts calculation with ´and´ **/
    for (i = 1; i < size; i += 2)
    {
        if (array[i] != '&')
            continue;

        // Do ´and´
        array[i - 1] = array[i - 1] && array[i + 1];

        // Override changes over the rest of array 
        for (j = i; j < size - 2; ++ j)
            array[j] = array[j + 2];

        size -= 2;
        i -= 2;
    }

    // Store first value as result -> Simpler to calculate with
    bool result = array[0];

    // Do ´or´
    for (i = 1; i < size; i += 2)
        result = result || array[i + 1];

    free (array);

    return result;
}

bool gba_calculation (const char* calc)
{
    /** Calculation with brackets, cuts bracket-piece and makes simple calculation **/
    if (_find_brackets (calc))
    {
        unsigned int new_calc_len = 1000;
        unsigned int i, j, w = 0;

        char new_calc[new_calc_len];

        for (i = 0; calc[i] != '\0'; ++ i)
        {
            // If bracket was found, breaks into a piece, calculates it, insert it with result
            if (calc[i] == '(')
            {
                int close_bracket = _find_close_bracket (calc, '(', ')', i);

                // TODO: ERROR Code !!!
                if (close_bracket == -1)
                    return 0;

                char* calc_bracket = _substr (calc, i + 1, close_bracket);
                bool  result       = gba_calculation (calc_bracket);
                char  str_result[16];

                // Free memory allocated from substr function!
                free (calc_bracket);

                // Convert bool "result" to string "str_result"
                sprintf (str_result, "%d", result);

                // override changes into "new_calc"
                for (j = 0; j < strlen (str_result); ++ j)
                    new_calc[w ++] = str_result[j];

                // Set iterator to last closed bracket found from function
                i = close_bracket;
            }
            else
                new_calc[w ++] = calc[i]; // if in no bracket, simply override chars of new_calc with calc
        }

        // Returning answer of new calculation string with inserted values from brackets
        return gba_calculation (_substr (new_calc, 0, w));
    }

    return gba_calculation_simple (calc); // Simple calculation if there are no brackets

}



void gna_consider_sign (char* str)
{
    unsigned int sign_counter;
    unsigned int str_len;
    unsigned int i, j, w = 0;
    char sign = '+';

    // Return if str is to short or has less or eq 1 sign
    if ((str_len = strlen(str)) <= 0)
        return;

    for (i = 0, sign_counter = 0; i < str_len; ++ i)
    {
        // If sign was found, calculate logic new sign & count up sign_counter
        if (str[i] == '+' || str[i] == '-')
        {
            sign_counter ++;
            sign = (str[i] == '-' && sign == '-') ? '+' : (str[i] == '-' && sign == '+') ? '-' : sign;
            continue;
        }

        // If just one or less signs were found reset counter (example: -12, +3, 5, 3, -90, ...)
        if (sign_counter <= 1)
        {
            sign_counter = 0;
            sign = '+';
            continue;
        }

        // Override changes (removing multiple signs)
        for (j = i, str[(w = i - sign_counter + 1) - 1] = sign; j < str_len; ++ j, ++ w)
            str[w] = str[j];

        sign_counter = 0;
        sign = '+';
    }

    // Delete garbage at end of string, fixes random values
    if (w != 0)
        memmove (&str[w], &str[str_len], str_len - w);
}

float* _gna_conv_calculation (const char* calc, unsigned int* size)
{
    unsigned int calc_len = strlen(calc);
    unsigned int array_i  = 0;
    unsigned int last_i   = 0;
    unsigned int i, j;

    // Find maximum element count by searching after computes 
    for (i = 0, *size = 1; i < strlen (calc); ++ i)
        if (calc[i] == '+' || calc[i] == '-' || calc[i] == '*' || calc[i] == '/')
            *size += 2;

    // Allocate memory to array containing values
    float* array = (float*) malloc (sizeof (float) * (*size));

    for (i = 0; i < calc_len; ++ i)
    {
        // Skips at beginning & if it is not a compute
        if ((calc[i] != '+' && calc[i] != '-' && calc[i] != '*' && calc[i] != '/') || i == 0)
            continue;

        char* value = _substr (calc, last_i, i);
        gna_consider_sign (value);

        // Insert value to float array
        array[array_i ++] = atof (value);

        // Insert compute to float array
        array[array_i ++] = calc[i];
        last_i = i + 1;

        // Moves index till no compute was found, or value was found
        // Used to allow using ´+´ or ´-´ after ´*´ or ´/´ (used for negative values, no brackets needed)
        for (j = 1; i + j < calc_len && (calc[i+j] == '+' || calc[i+j] == '-' || calc[i+j] == '*' || calc[i+j] == '/'); ++ j, ++ i);
    }

    // Insert value
    array[array_i ++] = atof (_substr (calc, last_i, i));

    return array;
}

float gna_calculation_simple (const char* calc)
{
    unsigned int i, j, size;
    float* array;

    // Converts cstring to an float array
    array = _gna_conv_calculation (calc, &size);

    /** Starts calculation with mul & div **/
    for (i = 1; i < size; i += 2)
    {
        char compute = array[i];

        if (compute != '*' && compute != '/')
            continue;

        float value_a = array[i - 1];
        float value_b = array[i + 1];

        // Calculating mul or div depending on compute
        array[i - 1] = (compute == '*') ? (value_a * value_b) : (value_a / value_b);

        // Override changes over the rest of the array 
        for (j = i; j < size - 2; ++ j)
            array[j] = array[j + 2];

        size -= 2;
        i -= 2;
    }

    /** Calculating with add & minus **/

    // Store first value as result -> Simpler to calculate with
    float result = array[0];

    // Sum of all values in float array
    for (i = 1; i < size; i += 2)
    {
        float value   = *(array + i + 1);
        char  compute = *(array + i);
        result += (compute == '+') ? value : (compute == '-') ? -value : 0;
    }

    free (array);
    
    return result;
}



float gna_calculation (const char* calc)
{
    /** Calculation with brackets, cuts bracket-piece and makes simple calculation **/
    if (_find_brackets (calc))
    {
        unsigned int new_calc_len = 1000;
        unsigned int i, j, w  = 0;

        char new_calc[new_calc_len];

        for (i = 0; calc[i] != '\0'; ++ i)
        {
            // If bracket was found, breaks into a piece, calculates it, insert it with result
            if (calc[i] == '(')
            {
                int   close_bracket = _find_close_bracket (calc, '(', ')', i);
                char* calc_bracket  = _substr (calc, i + 1, close_bracket);
                float result = gna_calculation (calc_bracket);
                char  str_result[16];

                // Free memory allocated from substr function!
                free (calc_bracket);

                // Convert float "result" to string "str_result"
                sprintf (str_result, "%f", result);

                // override changes into "new_calc"
                for (j = 0; j < strlen (str_result); ++ j)
                    new_calc[w ++] = str_result[j];

                // Set iterator to last closed bracket found from function
                i = close_bracket;
            }
            else
                new_calc[w ++] = calc[i]; // if in no bracket, simply override chars of new_calc with calc
        }

        char* fin_calc = _substr (new_calc, 0, w);    // Remove strange noise at end of string
        gna_consider_sign (fin_calc);                 // Calculating signs

        return gna_calculation (fin_calc);            // Returning answer of new calculation string with inserted values from brackets
    }

    return gna_calculation_simple (calc); // Simple calculation if there are no brackets
}













































void _gna_conv_to_registry_calculation (Value** array, const char* calc, size_t* size, Value (*fr_convert_to_value) (char*))
{
    size_t calc_len = strlen(calc);
    size_t array_i  = 0;
    size_t last_i   = 0;
    size_t i, j;

    char*  value;

    // Find maximum element count by searching after computes 
    for (i = 0, *size = 1; i < strlen (calc); ++ i)
        if (calc[i] == '+' || calc[i] == '-' || calc[i] == '*' || calc[i] == '/')
            *size += 2;

    // Allocate memory to array containing values
    (*array) = malloc (sizeof (Value) * (*size));

    for (i = 0; i < calc_len; ++ i)
    {
        // Skips at beginning & if it is not a compute
        if ((calc[i] != '+' && calc[i] != '-' && calc[i] != '*' && calc[i] != '/') || i == 0)
            continue;

        value = _substr (calc, last_i, i);
        gna_consider_sign (value);


        // Insert value to float array
        (*array)[array_i ++] = fr_convert_to_value (value);

        // Insert compute to float array
        (*array)[array_i ++] = VALUE_INT (calc[i]);

        last_i = i + 1;

        // Moves index till no compute was found, or value was found
        // Used to allow using ´+´ or ´-´ after ´*´ or ´/´ (used for negative values, no brackets needed)
        for (j = 1; i + j < calc_len && (calc[i+j] == '+' || calc[i+j] == '-' || calc[i+j] == '*' || calc[i+j] == '/'); ++ j, ++ i);
    }

    // Insert value
    (*array)[array_i ++] = fr_convert_to_value (_substr (calc, last_i, i));
}

Value gna_registry_calculation_simple (Registry** register_list, const char* calc, Value (*fr_convert_to_value) (char*), size_t (*var_add) (char*))
{
    size_t i, j, size;
    Value* array;

    // Converts cstring to an float array
    _gna_conv_to_registry_calculation (&array, calc, &size, fr_convert_to_value);

    bool has_found_term = 0;
    int  alloc_value    = 0;

    /** Starts calculation with mul & div **/
    for (i = 1; i < size; i += 2)
    {
        char compute = (intptr_t)array[i].value;

        if (compute != '*' && compute != '/')
        {
            has_found_term = 0;
            continue;
        }

        if (!has_found_term)
        {
            alloc_value = var_add("");
            fr_register_add (register_list, REGISTER_ALLOC (VALUE_INT (alloc_value), array[i - 1]));
            array[i - 1] = POINTER (alloc_value);
            has_found_term = 1;
        }

        // Calculating mul or div depending on compute
        if (compute == '*')
            fr_register_add (register_list, REGISTER_MUL (VALUE_INT (alloc_value), array[i + 1]));
        else
            fr_register_add (register_list, REGISTER_DIV (VALUE_INT (alloc_value), array[i + 1]));

        // Override changes over the rest of the array 
        for (j = i; j < size - 2; ++ j)
            array[j] = array[j + 2];

        size -= 2;
        i -= 2;
    }

    /** Calculating with add & minus **/


    // Store first value as result -> Simpler to calculate with
    size_t m_pos = var_add("");
    fr_register_add (register_list, REGISTER_ALLOC (VALUE_INT (m_pos), array[0]));

    // Sum of all values in float array
    for (i = 1; i < size; i += 2)
    {
        if ((intptr_t)array[i].value == '+')
            fr_register_add (register_list, REGISTER_ADD (VALUE_INT (m_pos), array[i + 1]));
        else
            fr_register_add (register_list, REGISTER_SUB (VALUE_INT (m_pos), array[i + 1]));
    }

    free (array);

    return POINTER (m_pos);
}
