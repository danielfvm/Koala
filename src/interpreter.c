#include "interpreter.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

void fr_register_create (Registry** register_list)
{
    (*register_list) = malloc (sizeof **register_list);
    (*register_list)[0] = NULL;
}

size_t fr_register_add (Registry** register_list, Register* reg)
{
    size_t i;

    for (i = 0; (*register_list)[i]; ++ i);

    (*register_list) = realloc (*register_list, sizeof (Register) * (i + 1));    
    
    (*register_list)[i] = reg;
    (*register_list)[i + 1] = NULL;

    return i;
}

/*
size_t fr_register_add_all (Registry** register_list, Register** regs)
{
    size_t position;

    while ((regs ++)[0])
        position = fr_register_add (register_list, regs[0]);

    return position;
}
*/

size_t fr_get_current_register_position (Registry** register_list)
{
    size_t i;

    for (i = 0; (*register_list)[i]; ++ i);

    return i;
}

/*
char* fr_get_register_type_as_name (Register* reg)
{
    switch (reg->reg_type)
    {
        case ALLOC: return "ALLOC";
        case OUT: return "OUT";
        case CIN: return "CIN";
        case EQ: return "EQ";
        case NEQ: return "NEQ";
        case BIG: return "BIG";
        case SMA: return "SMA";
        case BEQ: return "BEQ";
        case SEQ: return "SEQ";
        case JUMP: return "JUMP";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case SET: return "SET";
        case SYS: return "SYS";
        case NEG: return "NEG";
        case PUSH: return "PUSH";
        case POP: return "POP";
    }

    return "NONE";
}
*/

Value POINTER (int m_pointer)
{
    return (Value) { DT_POINTER, VALUE_TYPE_POINTER, (void*)(intptr_t) m_pointer };
}

Value POINTER_POINTER (int m_pointer)
{
    return (Value) { DT_POINTER_POINTER, VALUE_TYPE_POINTER_POINTER, (void*)(intptr_t) m_pointer };
}

Value INDEX (int m_pointer, int index)
{
    return (Value) { DT_POINTER, index, (void*)(intptr_t) m_pointer };
}

Value VALUE (byte data_type, void* value)
{
    return (Value) { data_type, VALUE_TYPE_VALUE, value };
}

Value VALUE_STR (void* value)
{
    char* str;
    strcpy (str = malloc (strlen (value) + 1), value);
    return (Value) { DT_STRING, VALUE_TYPE_VALUE, str };
}

Value VALUE_CHAR (char value)
{
    return (Value) { DT_CHAR, VALUE_TYPE_VALUE, (void*)(intptr_t) value };
}

Value VALUE_INT (int value)
{
    return (Value) { DT_INT, VALUE_TYPE_VALUE, (void*)(intptr_t) value };
}



Value VALUE_FLOAT (float value)
{
    return (Value) { DT_FLOAT, VALUE_TYPE_VALUE, (void*)(long long)(value * FLOAT_CONV_VALUE) };
}

Value VALUE_NULL ()
{
    return (Value) { DT_NONE, VALUE_TYPE_VALUE, NULL };
}

Value VALUE_VALUE (Value value)
{
    return (Value) { value.data_type, value.index, value.value };
}

Register* CREATE_REGISTER_1 (byte type, Value one)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type   = type;
    reg->reg_values[0] = one;
    return reg;
}

Register* CREATE_REGISTER_2 (byte type, Value one, Value two)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = type;
    reg->reg_values[0] = one;
    reg->reg_values[1] = two;
    return reg;
}

Register* CREATE_REGISTER_3 (byte type, Value one, Value two, Value three)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 3);
    reg->reg_type   = type;
    reg->reg_values[0] = one;
    reg->reg_values[1] = two;
    reg->reg_values[2] = three;
    return reg;
}

Register* REGISTER_ALLOC (Value m_value)
{
    return CREATE_REGISTER_2 (ALLOC, VALUE_NULL (), m_value);
}

Register* REGISTER_ADD (Value m_index, Value m_add)
{
    return CREATE_REGISTER_2 (ADD, m_index, m_add);
}

Register* REGISTER_SUB (Value m_index, Value m_sub)
{
    return CREATE_REGISTER_2 (SUB, m_index, m_sub);
}

Register* REGISTER_MUL (Value m_index, Value m_mul)
{
    return CREATE_REGISTER_2 (MUL, m_index, m_mul);
}

Register* REGISTER_MOD (Value m_index, Value m_mul)
{
    return CREATE_REGISTER_2 (MOD, m_index, m_mul);
}

Register* REGISTER_DIV (Value m_index, Value m_div)
{
    return CREATE_REGISTER_2 (DIV, m_index, m_div);
}

Register* REGISTER_CIN (Value m_index)
{
    return CREATE_REGISTER_1 (CIN, m_index);
}

Register* REGISTER_SET (Value m_index, Value m_value)
{
    return CREATE_REGISTER_2 (SET, m_index, m_value);
}

Register* REGISTER_EQ (Value m_value1, Value m_value2, Value not_position)
{
    return CREATE_REGISTER_3 (EQ, m_value1, m_value2, not_position);
}

Register* REGISTER_NEQ (Value m_value1, Value m_value2, Value not_position)
{
    return CREATE_REGISTER_3 (NEQ, m_value1, m_value2, not_position);
}

Register* REGISTER_SYS (Value cmd)
{
    return CREATE_REGISTER_1 (SYS, cmd);
}

Register* REGISTER_BIG (Value m_value1, Value m_value2, Value not_position)
{
    return CREATE_REGISTER_3 (BIG, m_value1, m_value2, not_position);
}

Register* REGISTER_SMA (Value m_value1, Value m_value2, Value not_position)
{
    return CREATE_REGISTER_3 (SMA, m_value1, m_value2, not_position);
}

Register* REGISTER_BEQ (Value m_value1, Value m_value2, Value not_position)
{
    return CREATE_REGISTER_3 (BEQ, m_value1, m_value2, not_position);
}

Register* REGISTER_SEQ (Value m_value1, Value m_value2, Value not_position)
{
    return CREATE_REGISTER_3 (SEQ, m_value1, m_value2, not_position);
}

Register* REGISTER_JUMP (Value position)
{
    return CREATE_REGISTER_1 (JUMP, position);
}

Register* REGISTER_OUT (Value m_index)
{
    return CREATE_REGISTER_1 (OUT, m_index);
}

Register* REGISTER_NEG (Value m_index)
{
    return CREATE_REGISTER_1 (NEG, m_index);
}

Register* REGISTER_PUSH (Value m_index)
{
    return CREATE_REGISTER_1 (PUSH, m_index);
}

Register* REGISTER_POP (Value m_index)
{
    return CREATE_REGISTER_1 (POP, m_index);
}

void fr_strcpy (char** dest, const char* src)
{
    int dest_size = *dest == NULL ? 0 : strlen (*dest);
    int src_size  = strlen (src);
    int i;

    (*dest) = *dest == NULL ? malloc (src_size + 1) : realloc (*dest, dest_size + src_size + 1);

    for (i = dest_size; i < dest_size + src_size; ++ i)
        (*dest)[i] = (char)src[i - dest_size];
    (*dest)[i] = '\0';
}

int fr_run (const Registry* register_list)
{
    Value fr_get_memory_value (Value value)
    {
        if (value.index == VALUE_TYPE_VALUE)
            return value;
        if (value.index == VALUE_TYPE_POINTER_POINTER)
            return register_list[(intptr_t)fr_get_memory_value ((register_list, register_list[(intptr_t) value.value]->reg_values[0])).value]->reg_values[0];
        if (value.index == VALUE_TYPE_POINTER)
            return register_list[(intptr_t) value.value]->reg_values[0];
        return VALUE_CHAR (((char*)register_list[(intptr_t) value.value]->reg_values[0].value)[value.index]);
    }

    void* fr_get_memory (Value value)
    {
        return fr_get_memory_value (value).value;
    }

    void fr_set_memory (Value value, void* new_value)
    {
        register_list[(intptr_t) fr_get_memory (value)]->reg_values[0].value = new_value;
    }

    void fr_set_data_type (Value value, int new_data_type)
    {
        register_list[(intptr_t) fr_get_memory (value)]->reg_values[0].data_type = new_data_type;
    }

    byte fr_get_data_type (Value value)
    {
        if (value.index == VALUE_TYPE_VALUE)
            return value.data_type;
        if (value.index == VALUE_TYPE_POINTER_POINTER)
            return register_list[(intptr_t)fr_get_memory (register_list[(intptr_t) value.value]->reg_values[0])]->reg_values[0].data_type;
        if (value.index == VALUE_TYPE_POINTER)
            return ((Value)register_list[(intptr_t) value.value]->reg_values[0]).data_type;
        return DT_CHAR;
    }

    if (!register_list)
    {
        fprintf (stderr, "´register_list´ is NULL!\n");
        return EXIT_FAILURE;
    }

    size_t size;

    // Calculating size of ´register_list´
    for (size = 0; register_list[size]; ++ size);

    // Stack
    Memory* stack = malloc (sizeof (Memory));
    size_t  stack_size = 0;


    // Start interpreting
    for (size_t i = 0; i < size && i >= 0; ++ i)
    {
        Register* reg = register_list[i];

        switch (reg->reg_type)
        {
            case PUSH:
            {
                stack[stack_size] = fr_get_memory_value (reg->reg_values[0]);
                stack = realloc (stack, sizeof (Memory) * ((++ stack_size) + 1));
                continue;
            }
            case POP:
            {
                if (stack_size == 0)
                    continue;
                stack = realloc (stack, sizeof (Memory) * (stack_size --));
                register_list[(intptr_t)fr_get_memory (reg->reg_values[0])]->reg_values[0] = stack[stack_size];
                continue;
            }
            case ALLOC:
            {
                if ((reg->reg_values[0].data_type == DT_STRING || fr_get_data_type (reg->reg_values[1]) == DT_STRING) && reg->reg_values[0].value)
                    free (reg->reg_values[0].value);
                if (fr_get_data_type (reg->reg_values[1]) == DT_STRING)
                {
                    char* text = fr_get_memory_value (reg->reg_values[1]).value;
                    strcpy (reg->reg_values[0].value = malloc (strlen (text) + 1), text);
                    reg->reg_values[0].data_type = DT_STRING;
                }
                else
                    reg->reg_values[0] = fr_get_memory_value (reg->reg_values[1]);
                continue;
            }
            case SET:
            {
                fr_set_memory    (reg->reg_values[0], fr_get_memory (reg->reg_values[1]));
                fr_set_data_type (reg->reg_values[0], fr_get_data_type (reg->reg_values[1]));
                continue;
            }
            case NEG:
            {
                fr_set_memory (reg->reg_values[0],(void*)(intptr_t)(fr_get_memory (POINTER ((intptr_t)fr_get_memory (reg->reg_values[0]))) ? 0 : 1));
                continue;
            }
            case ADD:
            {
                intptr_t m_value     = (intptr_t) fr_get_memory (reg->reg_values[0]);
                intptr_t m_value_add = (intptr_t) fr_get_memory (reg->reg_values[1]);
                void* new_value;

                if (register_list[m_value]->reg_values[0].data_type == DT_STRING)
                {
                    char* value    = register_list[m_value]->reg_values[0].value;
                    byte data_type = fr_get_data_type (reg->reg_values[1]);

                    void** m_value = &register_list[(intptr_t) fr_get_memory (reg->reg_values[0])]->reg_values[0].value;

                    if (data_type == DT_STRING)
                    {
                        char* value_add = fr_get_memory (reg->reg_values[1]);
                        (*m_value) = realloc (*m_value, strlen (value) + strlen (value_add) + 1);
                        strcat (*m_value, value_add); 
                    }
                    else if (data_type == DT_CHAR)
                    {
                        int  size = strlen (*m_value);
                        (*m_value) = realloc (*m_value, strlen (*m_value) + 2);
                        ((char*) *m_value)[size] = m_value_add;
                        ((char*) *m_value)[size + 1] = '\0';
                    }
                    else if (data_type == DT_INT)
                    {
                        char* str_num = malloc (10 + 1);
                        sprintf (str_num, "%d", (int)(intptr_t) m_value_add);
                        int old_size = strlen (*m_value);
                        int new_size = old_size + strlen (str_num);
                        (*m_value) = realloc (*m_value, new_size + 1);
                        int j;
                        for (j = old_size; j < new_size; ++ j)
                            ((char*)(*m_value))[j] = str_num[j - old_size];
                        ((char*)(*m_value))[j] = str_num[j - old_size];
                        free (str_num);
                    }
                    else if (data_type == DT_FLOAT)
                    {
                        char* str_num = malloc (38 + 1);
                        sprintf (str_num, "%f", (float)(intptr_t) m_value_add / FLOAT_CONV_VALUE);
                        int old_size = strlen (*m_value);
                        int new_size = old_size + strlen (str_num);
                        (*m_value) = realloc (*m_value, new_size + 1);
                        int j;
                        for (j = old_size; j < new_size; ++ j)
                            ((char*)(*m_value))[j] = str_num[j - old_size];
                        ((char*)(*m_value))[j] = str_num[j - old_size];
                        free (str_num);
                    }
                    continue;
                }

                if (register_list[m_value]->reg_values[0].data_type == DT_FLOAT && fr_get_data_type (reg->reg_values[1]) != DT_FLOAT)
                    m_value_add *= FLOAT_CONV_VALUE;
                else if (register_list[m_value]->reg_values[0].data_type != DT_FLOAT && fr_get_data_type (reg->reg_values[1]) == DT_FLOAT)
                    m_value_add /= FLOAT_CONV_VALUE;

                new_value = register_list[m_value]->reg_values[0].value + m_value_add;
                fr_set_memory (reg->reg_values[0], (void*) new_value);
                continue;
            }
            case SUB:
            {
                intptr_t m_value     = (intptr_t) fr_get_memory (reg->reg_values[0]);
                intptr_t m_value_sub = (intptr_t) fr_get_memory (reg->reg_values[1]);

                if (register_list[m_value]->reg_values[0].data_type == DT_FLOAT && fr_get_data_type (reg->reg_values[1]) != DT_FLOAT)
                    m_value_sub *= FLOAT_CONV_VALUE;
                else if (register_list[m_value]->reg_values[0].data_type != DT_FLOAT && fr_get_data_type (reg->reg_values[1]) == DT_FLOAT)
                    m_value_sub /= FLOAT_CONV_VALUE;

                fr_set_memory (reg->reg_values[0], (void*) (register_list[m_value]->reg_values[0].value - m_value_sub));
                continue;
            }
            case MUL:
            {
                void** value = &register_list[(intptr_t) fr_get_memory (reg->reg_values[0])]->reg_values[0].value;
                intptr_t m_value_mul = (intptr_t) fr_get_memory (reg->reg_values[1]);
                byte     data_type_add = fr_get_data_type (reg->reg_values[1]);
                byte     data_type     = register_list[(intptr_t) fr_get_memory (reg->reg_values[0])]->reg_values[0].data_type;

                size_t i; 

                if ((data_type == DT_STRING && data_type_add == DT_INT) || (data_type == DT_INT && data_type_add == DT_STRING))
                {
                    intptr_t text_mul = (data_type == DT_STRING) ? (intptr_t) m_value_mul : (intptr_t)(*value);
                    text_mul = (int) text_mul < 0 ? 0 : text_mul;

                    char* text_copy = NULL;
                    fr_strcpy (&text_copy, (data_type != DT_STRING) ? (char*) m_value_mul : (char*)(*value));

                    if (data_type == DT_STRING)
                        free (*value);
                    (*value) = NULL;

                    for (; text_mul > 0; -- text_mul)
                        fr_strcpy ((char**) value, text_copy);
                    free (text_copy);

                    fr_set_data_type (reg->reg_values[0], DT_STRING);
                }
                else if ((data_type == DT_CHAR && data_type_add == DT_INT) || (data_type == DT_INT && data_type_add == DT_CHAR))
                {
                    intptr_t new_text_size = (data_type == DT_CHAR) ? m_value_mul : (intptr_t)(*value);
                    new_text_size = (int) new_text_size < 0 ? 0 : new_text_size;

                    char char_copy = (data_type != DT_CHAR) ? m_value_mul : (intptr_t)(*value);

                    (*value) = malloc (new_text_size + 1);
                    for (i = 0; i < new_text_size; ++ i)
                        ((char*)(*value))[i] = char_copy;
                    ((char*)(*value))[i] = '\0';

                    fr_set_data_type (reg->reg_values[0], DT_STRING);
                }
                else if (data_type == DT_FLOAT && data_type_add != DT_FLOAT)
                    (*value) = (void*)(intptr_t)(((float)(intptr_t)(*value) * (m_value_mul * FLOAT_CONV_VALUE)) / FLOAT_CONV_VALUE);
                else if (data_type != DT_FLOAT && data_type_add == DT_FLOAT)
                    (*value) = (void*)(intptr_t)((float)(intptr_t)(*value) * (m_value_mul / FLOAT_CONV_VALUE));
                else if (data_type == DT_FLOAT && data_type_add == DT_FLOAT)
                    (*value) = (void*)(intptr_t)((((float)(intptr_t)(*value) / FLOAT_CONV_VALUE) * (m_value_mul / FLOAT_CONV_VALUE)) * FLOAT_CONV_VALUE);
                else
                    (*value) = (void*)(((intptr_t)(*value)) * m_value_mul);
                continue;
            }
            case MOD:
            {
                void** value = &register_list[(intptr_t) fr_get_memory (reg->reg_values[0])]->reg_values[0].value;
                intptr_t m_value_mod = (intptr_t) fr_get_memory (reg->reg_values[1]);
                byte data_type_add = fr_get_data_type (reg->reg_values[1]);
                byte data_type     = register_list[(intptr_t) fr_get_memory (reg->reg_values[0])]->reg_values[0].data_type;

                (*value) = (void*)((intptr_t)(*value) % (data_type_add == DT_FLOAT ? (intptr_t)(m_value_mod / FLOAT_CONV_VALUE) : m_value_mod));
                continue;
            }
            case DIV:
            {
                intptr_t m_value       = (intptr_t) fr_get_memory (reg->reg_values[0]);
                intptr_t m_value_div   = (intptr_t) fr_get_memory (reg->reg_values[1]);
                byte data_type_add = fr_get_data_type (reg->reg_values[1]);
                byte data_type     = register_list[m_value]->reg_values[0].data_type;

                if (data_type == DT_FLOAT && data_type_add != DT_FLOAT) 
                    fr_set_memory (reg->reg_values[0], (void*)(intptr_t)((((intptr_t) register_list[m_value]->reg_values[0].value / FLOAT_CONV_VALUE) / m_value_div) * FLOAT_CONV_VALUE));
                else if (data_type != DT_FLOAT && data_type_add == DT_FLOAT)
                    fr_set_memory (reg->reg_values[0], (void*)(intptr_t)((intptr_t) register_list[m_value]->reg_values[0].value / (m_value_div / FLOAT_CONV_VALUE)));
                else if (data_type == DT_FLOAT && data_type_add == DT_FLOAT)
                    fr_set_memory (reg->reg_values[0], (void*)(intptr_t)((((intptr_t) register_list[m_value]->reg_values[0].value / FLOAT_CONV_VALUE) / ((float) m_value_div / FLOAT_CONV_VALUE)) * FLOAT_CONV_VALUE));
                else
                    fr_set_memory (reg->reg_values[0], (void*) ((intptr_t) register_list[m_value]->reg_values[0].value / m_value_div));
                continue;
            }
            case EQ:
            {
                if (fr_get_memory (reg->reg_values[0]) != fr_get_memory (reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (reg->reg_values[2]) - 1;
                continue;
            }
            case NEQ:
            {
                if ((int)(intptr_t)fr_get_memory (reg->reg_values[0]) == (int)(intptr_t)fr_get_memory (reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (reg->reg_values[2]) - 1;
                continue;
            }
            case BEQ:
            {
                fr_set_memory (reg->reg_values[2], (void*)(intptr_t)((int)(intptr_t)fr_get_memory (reg->reg_values[0]) >= (int)(intptr_t)fr_get_memory (reg->reg_values[1])));
                continue;
            }
            case SEQ:
            {
                fr_set_memory (reg->reg_values[2], (void*)(intptr_t)((int)(intptr_t)fr_get_memory (reg->reg_values[0]) <= (int)(intptr_t)fr_get_memory (reg->reg_values[1])));
                continue;
            }
            case BIG:
            {
                fr_set_memory (reg->reg_values[2], (void*)(intptr_t)((int)(intptr_t)fr_get_memory (reg->reg_values[0]) > (int)(intptr_t)fr_get_memory (reg->reg_values[1])));
                continue;
            }
            case SMA:
            {
                fr_set_memory (reg->reg_values[2], (void*)(intptr_t)((int)(intptr_t)fr_get_memory (reg->reg_values[0]) < (int)(intptr_t)fr_get_memory (reg->reg_values[1])));
                continue;
            }
            case OUT:
            {
                byte m_type = fr_get_data_type (reg->reg_values[0]); 
                if (m_type == DT_STRING)
                    printf ("%s", (char*) fr_get_memory (reg->reg_values[0]));
                else if (m_type == DT_INT)
                    printf ("%d", (intptr_t) fr_get_memory (reg->reg_values[0]));
                else if (m_type == DT_CHAR)
                    printf ("%c", (intptr_t) fr_get_memory (reg->reg_values[0]));
                else if (m_type == DT_FLOAT)
                    printf ("%f", (intptr_t) fr_get_memory (reg->reg_values[0]) / FLOAT_CONV_VALUE);
                continue;
            }
            case CIN:
            {
                byte   m_type  =  register_list[(intptr_t) fr_get_memory (reg->reg_values[0])]->reg_values[0].data_type; 
                void** m_value = &register_list[(intptr_t) fr_get_memory (reg->reg_values[0])]->reg_values[0].value;

                if (m_type == DT_STRING)
                {
                    size_t buffer_size = 100000;
                    char*  buffer = malloc (buffer_size);

                    getline (&buffer, &buffer_size, stdin);

                    buffer[strlen (buffer) - 1] = '\0';
                    *m_value = buffer;

                    free (buffer);
                }
                else if (m_type == DT_INT)
                {
                    scanf ("%d", m_value);
                    getchar();
                }
                else if (m_type == DT_CHAR)
                {
                    scanf ("%c", m_value);
                    getchar();
                }
                else if (m_type == DT_FLOAT)
                {
                    float value;
                    scanf ("%f", &value);
                    *m_value = (void*)(intptr_t)(value * FLOAT_CONV_VALUE);
                    getchar();
                }
                continue;
            }
            case JUMP:
            {
                i = (intptr_t) fr_get_memory (reg->reg_values[0]) - 1;
                continue;
            }
            case SYS:
            {
                system ((char*) fr_get_memory (reg->reg_values[0]));
                continue;
            }
        }
    }

    for (size_t i = 0; register_list[i]; ++ i)
    {
        free (register_list[i]->reg_values);
        free (register_list[i]);
    }

    free (stack);

    return EXIT_SUCCESS;
}
