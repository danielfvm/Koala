#include "gnumber.h"
#include "parser.h"

#include "gnumber.h"

#include <string.h>
#include <unistd.h>

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

void _gna_conv_to_registry_calculation (Value** array, const char* calc, size_t* size, Value (*fr_convert_to_value) (char*))
{
    size_t calc_len = strlen(calc);
    size_t array_i  = 0;
    size_t last_i   = 0;
    size_t i, j;

    char* value;

    bool in_string = false;
    bool in_char   = false;
    int  in_parentheses_brackets  = 0;
    int  in_square_brackets = 0;
    int  in_curly_brackets = 0;

    // Find maximum element count by searching after computes 
    for (i = 0, *size = 1; i < strlen (calc); ++ i)
    {
        // continue if double '\'
        if (calc[i] == '\\' && i >= 1 && calc[i-1] == '\\' && ++ i)
            continue;

        if (!in_char && calc[i] == '"' && (i == 0 || calc[i-1] != '\\'))
            in_string = !in_string;

        if (!in_string && calc[i] == '\'' && (i == 0 || calc[i-1] != '\\'))
            in_char = !in_char;

        if (!in_char && !in_string && calc[i] == '{')
            in_curly_brackets ++;
        if (!in_char && !in_string && calc[i] == '}')
            in_curly_brackets --;

        if (!in_char && !in_string && calc[i] == '(')
            in_parentheses_brackets ++;
        if (!in_char && !in_string && calc[i] == ')')
            in_parentheses_brackets --;

        if (!in_char && !in_string && calc[i] == '[')
            in_square_brackets ++;
        if (!in_char && !in_string && calc[i] == ']')
            in_square_brackets --;

        if (!in_string && 
            !in_char && 
            !in_curly_brackets && 
            !in_parentheses_brackets && 
            !in_square_brackets && 
            (calc[i] == '+' || calc[i] == '-' || calc[i] == '*' || calc[i] == '/' || calc[i] == '%'))
                *size += 2;
    }

    // Allocate memory to array containing values
    (*array) = malloc (sizeof (Value) * (*size));

    in_string = false;
    in_char   = false;
    in_curly_brackets = 0;

    for (i = 0; i < calc_len; ++ i)
    {
        // continue if double '\' TODO: DOES NOT WORK!
        if (calc[i] == '\\' && i >= 1 && calc[i-1] == '\\' && ++ i)
            continue;

        if (!in_char && calc[i] == '"' && (i == 0 || calc[i-1] != '\\'))
            in_string = !in_string;

        if (!in_string && calc[i] == '\'' && (i == 0 || calc[i-1] != '\\'))
            in_char = !in_char;

        if (!in_char && !in_string && calc[i] == '{')
            in_curly_brackets ++;
        if (!in_char && !in_string && calc[i] == '}')
            in_curly_brackets --;

        if (!in_char && !in_string && calc[i] == '(')
            in_parentheses_brackets ++;
        if (!in_char && !in_string && calc[i] == ')')
            in_parentheses_brackets --;

        if (!in_char && !in_string && calc[i] == '[')
            in_square_brackets ++;
        if (!in_char && !in_string && calc[i] == ']')
            in_square_brackets --;

        // Skips at beginning & if it is not a compute
        if ((calc[i] != '+' && calc[i] != '-' && calc[i] != '*' && calc[i] != '/' && calc[i] != '%') || i == 0)
            continue;

        if (in_string || in_char || in_curly_brackets || in_parentheses_brackets || in_square_brackets)
            continue;

        value = kl_util_substr (calc, last_i, i);
        gna_consider_sign (value);

        // Insert value to float array
        (*array)[array_i ++] = fr_convert_to_value (value);

        // Insert compute to float array
        (*array)[array_i ++] = VALUE_INT (calc[i]);

        last_i = i + 1;

        // Moves index till no compute was found, or value was found
        // Used to allow using ´+´ or ´-´ after ´*´ or ´/´ (used for negative values, no brackets needed)
        for (
            j = 1; 
            i + j < calc_len && (calc[i+j] == '+' || calc[i+j] == '-' || calc[i+j] == '*' || calc[i+j] == '/' || calc[i+j] == '%'); 
            ++ j, ++ i
        );
    }

    // Insert value
    (*array)[array_i ++] = fr_convert_to_value (kl_util_substr (calc, last_i, i));
}

Value gna_registry_calculation_simple (Registry** register_list, const char* calc, Value (*fr_convert_to_value) (char*))
{
    size_t i, j, size;
    Value* array;

    // Converts cstring to an float array
    _gna_conv_to_registry_calculation (&array, calc, &size, fr_convert_to_value);

    bool has_found_term = false;
    int  alloc_value    = 0;

    /** Starts calculation with mul & div **/
    for (i = 1; i < size; i += 2)
    {
        char compute = (intptr_t)array[i].value;

        if (compute != '*' && compute != '/' && compute != '%')
        {
            has_found_term = false;
            continue;
        }

        if (!has_found_term)
        {
            alloc_value = kl_intp_register_add (register_list, REGISTER_ALLOC (array[i - 1]));
            array[i - 1] = POINTER (alloc_value);
            has_found_term = true;
        }

        // Calculating mul or div depending on compute
        if (compute == '*')
            kl_intp_register_add (register_list, REGISTER_MUL (VALUE_INT (alloc_value), array[i + 1]));
        else if (compute == '%')
            kl_intp_register_add (register_list, REGISTER_MOD (VALUE_INT (alloc_value), array[i + 1]));
        else if (compute == '/')
            kl_intp_register_add (register_list, REGISTER_DIV (VALUE_INT (alloc_value), array[i + 1]));

        // Override changes over the rest of the array 
        for (j = i; j < size - 2; ++ j)
            array[j] = array[j + 2];

        size -= 2;
        i -= 2;
    }

    /** Calculating with add & minus **/

    // Store first value as result -> Simpler to calculate with
    size_t m_pos = kl_intp_register_add (register_list, REGISTER_ALLOC (array[0]));

    // Sum of all values in float array
    for (i = 1; i < size; i += 2)
    {
        if ((intptr_t)array[i].value == '+')
            kl_intp_register_add (register_list, REGISTER_ADD (VALUE_INT (m_pos), array[i + 1]));
        else
            kl_intp_register_add (register_list, REGISTER_SUB (VALUE_INT (m_pos), array[i + 1]));
    }

    free (array);

    return POINTER (m_pos);
}

void _gna_conv_to_registry_boolean_algebra (Value** array, const char* calc, size_t* size, Value (*fr_convert_to_value) (char*))
{
    size_t calc_len = strlen(calc);
    size_t array_i  = 0;
    size_t last_i   = 0;
    size_t i, j;

    bool in_string = false;
    bool in_char   = false;
    int  in_parentheses_brackets = 0;
    int  in_square_brackets = 0;
    int  in_curly_brackets = 0;

    // Find maximum element count by searching after computes 
    for (i = 0, *size = 1; i < strlen (calc); ++ i)
    {
        // continue if double '\'
        if (calc[i] == '\\' && i >= 1 && calc[i-1] == '\\' && ++ i)
            continue;

        if (!in_char && calc[i] == '"' && (i == 0 || calc[i-1] != '\\'))
            in_string = !in_string;

        if (!in_string && calc[i] == '\'' && (i == 0 || calc[i-1] != '\\'))
            in_char = !in_char;

        if (!in_char && !in_string && calc[i] == '{')
            in_curly_brackets ++;
        if (!in_char && !in_string && calc[i] == '}')
            in_curly_brackets --;

        if (!in_char && !in_string && calc[i] == '(')
            in_parentheses_brackets ++;
        if (!in_char && !in_string && calc[i] == ')')
            in_parentheses_brackets --;

        if (!in_char && !in_string && calc[i] == '[')
            in_square_brackets ++;
        if (!in_char && !in_string && calc[i] == ']')
            in_square_brackets --;

        if (!in_string && 
            !in_char && 
            !in_curly_brackets && 
            !in_parentheses_brackets && 
            !in_square_brackets && 
            (calc[i] == '&' || calc[i] == '|'))
                *size += 2;
    }

    // Allocate memory to array containing values
    (*array) = malloc (sizeof (Value) * (*size));

    in_string = false;
    in_char   = false;
    in_curly_brackets = 0;
    in_parentheses_brackets  = 0;
    in_square_brackets = 0;

    for (i = 0; i < calc_len; ++ i)
    {
        // continue if double '\' TODO: DOES NOT WORK!
        if (calc[i] == '\\' && i >= 1 && calc[i-1] == '\\' && ++ i)
            continue;

        if (!in_char && calc[i] == '"' && (i == 0 || calc[i-1] != '\\'))
            in_string = !in_string;

        if (!in_string && calc[i] == '\'' && (i == 0 || calc[i-1] != '\\'))
            in_char = !in_char;

        if (!in_char && !in_string && calc[i] == '{')
            in_curly_brackets ++;
        if (!in_char && !in_string && calc[i] == '}')
            in_curly_brackets --;

        if (!in_char && !in_string && calc[i] == '(')
            in_parentheses_brackets ++;
        if (!in_char && !in_string && calc[i] == ')')
            in_parentheses_brackets --;

        if (!in_char && !in_string && calc[i] == '[')
            in_square_brackets ++;
        if (!in_char && !in_string && calc[i] == ']')
            in_square_brackets --;

        // Skips at beginning and if it is not a compute
        if (calc[i] != '&' && calc[i] != '|')
            continue;
        if (i == 0)
            error ("Compute ´%c´ cannot stand at the beginning!", (void*)(intptr_t)calc[i]);

        if (in_string || in_char || in_curly_brackets || in_parentheses_brackets || in_square_brackets)
            continue;

        // Insert value to float array
        (*array)[array_i ++] = fr_convert_to_value (kl_util_substr (calc, last_i, i));

        // Insert compute to float array
        (*array)[array_i ++] = VALUE_INT (calc[i]);

        last_i = i + 1;

        // Moves index till no compute was found, or value was found
        for (j = 1; i + j < calc_len && (calc[i+j] == '&' || calc[i+j] == '|'); ++ j, ++ i);
    }

    // Insert value
    (*array)[array_i ++] = fr_convert_to_value (kl_util_substr (calc, last_i, i));
}

Value gna_registry_boolean_algebra (Registry** register_list, const char* calc, Value (*fr_convert_to_value) (char*))
{
    size_t i, j, size;
    Value* array;

    // Converts cstring to an float array
    _gna_conv_to_registry_boolean_algebra (&array, calc, &size, fr_convert_to_value);

    bool has_found_term = 0;
    int  alloc_value    = 0;

    /** Starts calculation with AND ´&´ **/
    for (i = 1; i < size; i += 2)
    {
        char compute = (intptr_t)array[i].value;

        if (compute != '&')
        {
            has_found_term = 0;
            continue;
        }

        if (!has_found_term)
        {
            alloc_value = kl_intp_register_add (register_list, REGISTER_ALLOC (array[i - 1]));
            array[i - 1] = POINTER (alloc_value);
            has_found_term = 1;
        }

        // Calculating mul or div depending on compute
        if (compute == '&')
            kl_intp_register_add (register_list, REGISTER_AND (VALUE_INT (alloc_value), array[i + 1]));
        else
            ; // TODO: Error!!! Unknown Compute!

        // Override changes over the rest of the array 
        for (j = i; j < size - 2; ++ j)
            array[j] = array[j + 2];

        size -= 2;
        i -= 2;
    }

    /** Calculating with OR ´|´ **/

    // Store first value as result -> Simpler to calculate with
    size_t m_pos = kl_intp_register_add (register_list, REGISTER_ALLOC (array[0]));

    // Sum of all values in float array
    for (i = 1; i < size; i += 2)
    {
        if ((intptr_t)array[i].value == '|')
            kl_intp_register_add (register_list, REGISTER_OR (VALUE_INT (m_pos), array[i + 1]));
        else
            ; // TODO: Error!!! Unknown compute!
    }

    free (array);

    return POINTER (m_pos);
}
