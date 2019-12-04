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

size_t fr_register_add_all (Registry** register_list, Register** regs)
{
    size_t position;

    while ((regs ++)[0])
        position = fr_register_add (register_list, regs[0]);

    return position;
}

size_t fr_get_current_register_position (Registry** register_list)
{
    size_t i;

    for (i = 0; (*register_list)[i]; ++ i);

    return i;
}

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
    return (Value) { DT_STRING, VALUE_TYPE_VALUE, value };
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

Value VALUE_VALUE (Value value)
{
    return (Value) { value.data_type, value.index, value.value };
}

Register* REGISTER_ALLOC (Value m_value)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = ALLOC;
 // reg->reg_values[0] = /* Stored Value */;
    reg->reg_values[1] = m_value;
    return reg;
}

Register* REGISTER_ADD (Value m_index, Value m_add)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = ADD;
    reg->reg_values[0] = m_index;
    reg->reg_values[1] = m_add;
    return reg;
}

Register* REGISTER_SUB (Value m_index, Value m_sub)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = SUB;
    reg->reg_values[0] = m_index;
    reg->reg_values[1] = m_sub;
    return reg;
}

Register* REGISTER_MUL (Value m_index, Value m_mul)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = MUL;
    reg->reg_values[0] = m_index;
    reg->reg_values[1] = m_mul;
    return reg;
}

Register* REGISTER_CIN (Value m_index)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type   = CIN;
    reg->reg_values[0] = m_index;
    return reg;
}

Register* REGISTER_DIV (Value m_index, Value m_div)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = DIV;
    reg->reg_values[0] = m_index;
    reg->reg_values[1] = m_div;
    return reg;
}

Register* REGISTER_SET (Value m_index, Value m_value)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = SET;
    reg->reg_values[0] = m_index;
    reg->reg_values[1] = m_value;
    return reg;
}

Register* REGISTER_EQ (Value m_value1, Value m_value2, Value not_position)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 3);
    reg->reg_type   = EQ;
    reg->reg_values[0] = m_value1;
    reg->reg_values[1] = m_value2;
    reg->reg_values[2] = not_position;
    return reg;
}

Register* REGISTER_NEQ (Value m_value1, Value m_value2, Value not_position)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 3);
    reg->reg_type   = NEQ;
    reg->reg_values[0] = m_value1;
    reg->reg_values[1] = m_value2;
    reg->reg_values[2] = not_position;
    return reg;
}

Register* REGISTER_SYS (Value cmd)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type   = SYS;
    reg->reg_values[0] = cmd;
    return reg;
}

Register* REGISTER_BIG (Value m_value1, Value m_value2, Value not_position)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 3);
    reg->reg_type   = BIG;
    reg->reg_values[0] = m_value1;
    reg->reg_values[1] = m_value2;
    reg->reg_values[2] = not_position;
    return reg;
}

Register* REGISTER_SMA (Value m_value1, Value m_value2, Value not_position)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 3);
    reg->reg_type   = SMA;
    reg->reg_values[0] = m_value1;
    reg->reg_values[1] = m_value2;
    reg->reg_values[2] = not_position;
    return reg;
}

Register* REGISTER_BEQ (Value m_value1, Value m_value2, Value not_position)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 3);
    reg->reg_type   = BEQ;
    reg->reg_values[0] = m_value1;
    reg->reg_values[1] = m_value2;
    reg->reg_values[2] = not_position;
    return reg;
}

Register* REGISTER_SEQ (Value m_value1, Value m_value2, Value not_position)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 3);
    reg->reg_type   = SEQ;
    reg->reg_values[0] = m_value1;
    reg->reg_values[1] = m_value2;
    reg->reg_values[2] = not_position;
    return reg;
}

Register* REGISTER_JMP (Value position)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type   = JUMP;
    reg->reg_values[0] = position;
    return reg;
}

Register* REGISTER_OUT (Value m_index)
{
    Register* reg = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type = OUT;
    reg->reg_values[0] = m_index;
    return reg;
}

Register* REGISTER_NEG (Value m_index)
{
    Register* reg = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type = NEG;
    reg->reg_values[0] = m_index;
    return reg;
}

Register* REGISTER_PUSH (Value m_index)
{
    Register* reg = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type = PUSH;
    reg->reg_values[0] = m_index;
    return reg;
}

Register* REGISTER_POP (Value m_index)
{
    Register* reg = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type = POP;
    reg->reg_values[0] = m_index;
    return reg;
}

int fr_run (const Registry* register_list)
{
    Value fr_get_memory_value (Value value)
    {
        if (value.index == VALUE_TYPE_VALUE)
            return value;
        if (value.index == VALUE_TYPE_POINTER_POINTER)
            return register_list[(intptr_t)fr_get_memory_value ((register_list, register_list[(intptr_t) value.value]->reg_values[0])).value]->reg_values[0];
        return register_list[(intptr_t) value.value]->reg_values[0];
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
        return ((Value)register_list[(intptr_t) value.value]->reg_values[0]).data_type;
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
                stack[stack_size ++] = fr_get_memory_value (reg->reg_values[0]);
//                printf ("PUSH>>>%d\n",fr_get_memory (reg->reg_values[0]));
                stack = realloc (stack, sizeof (Memory) * (stack_size + 1));
                continue;
            }
            case POP:
            {
                if (stack_size == 0)
                    continue;
//                printf ("POP>>>%d\n",stack[stack_size-1].value);
                stack = realloc (stack, sizeof (Memory) * stack_size);
                register_list[(intptr_t)fr_get_memory (reg->reg_values[0])]->reg_values[0] = stack[stack_size -= 1];
                continue;
            }
            case ALLOC:
            {
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

                if (register_list[m_value]->reg_values[0].data_type == DT_FLOAT && fr_get_data_type (reg->reg_values[1]) != DT_FLOAT)
                    m_value_add *= FLOAT_CONV_VALUE;
                else if (register_list[m_value]->reg_values[0].data_type != DT_FLOAT && fr_get_data_type (reg->reg_values[1]) == DT_FLOAT)
                    m_value_add /= FLOAT_CONV_VALUE;

                if (register_list[m_value]->reg_values[0].data_type == DT_STRING)
                {
                    char* value    = register_list[m_value]->reg_values[0].value;
                    byte data_type = fr_get_data_type (reg->reg_values[1]);

                    if (data_type == DT_STRING)
                    {
                        char* value_add = fr_get_memory (reg->reg_values[1]);
                        new_value = malloc (strlen (value) + strlen (value_add) + 1);
                        strcpy (new_value, value);
                        strcat (new_value, value_add);
                    }
                    else if (data_type == DT_CHAR)
                    {
                        size_t ssize = strlen (value);
                        new_value = malloc (ssize + 2);
                        strcpy (new_value, value);
                        ((char*) new_value)[ssize] = (intptr_t) fr_get_memory (reg->reg_values[1]);
                        ((char*) new_value)[ssize + 1] = '\0';
                    }
                    else if (data_type == DT_INT)
                    {
                        char* text = malloc (strlen (value) + 20);
                        sprintf (text, "%s%d", value, (intptr_t) fr_get_memory (reg->reg_values[1]));
                        strcpy (new_value = malloc (strlen (text) + 1), text);
                        free (text);
                    }
                    else if (data_type == DT_FLOAT)
                    {
                        char* text = malloc (strlen (value) + 20);
                        sprintf (text, "%s%f", value, ((intptr_t) fr_get_memory (reg->reg_values[1])) / FLOAT_CONV_VALUE);
                        strcpy (new_value = malloc (strlen (text) + 1), text);
                        free (text);
                    }
                }
                else
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
                intptr_t m_value     = (intptr_t) fr_get_memory (reg->reg_values[0]);
                intptr_t m_value_mul = (intptr_t) fr_get_memory (reg->reg_values[1]);
                byte     data_type_add = fr_get_data_type (reg->reg_values[1]);
                byte     data_type     = register_list[m_value]->reg_values[0].data_type;

                size_t i; 

                if ((data_type == DT_STRING && data_type_add == DT_INT) || (data_type == DT_INT && data_type_add == DT_STRING))
                {
                    intptr_t text_mul  = (data_type == DT_STRING) ? (intptr_t) m_value_mul : (intptr_t) register_list[m_value]->reg_values[0].value;
                    text_mul = (int)text_mul < 0 ? 0 : text_mul;

                    char*    text_copy = (data_type != DT_STRING) ? (char*)    m_value_mul : (char*)    register_list[m_value]->reg_values[0].value;
                    char*    new_text  = malloc (strlen (text_copy) * text_mul + 1);

                    if (text_mul > 0)
                        strcpy (new_text, text_copy);
                    else
                        new_text[0] = '\0';

                    for (i = 1; i < text_mul; ++ i)
                        strcat (new_text, text_copy);

                    fr_set_memory (reg->reg_values[0], (void*) new_text);
                    fr_set_data_type (reg->reg_values[0], DT_STRING);
                }
                else if ((data_type == DT_CHAR && data_type_add == DT_INT) || (data_type == DT_INT && data_type_add == DT_CHAR))
                {
                    intptr_t new_text_size = (data_type == DT_CHAR) ? m_value_mul : (intptr_t) register_list[m_value]->reg_values[0].value;
                    new_text_size = (int)new_text_size < 0 ? 0 : new_text_size;

                    char   char_copy       = (data_type != DT_CHAR) ? m_value_mul : (intptr_t) register_list[m_value]->reg_values[0].value;
                    char*  new_text = malloc (new_text_size + 1);

                    for (i = 0; i < new_text_size; ++ i)
                        new_text[i] = char_copy;
                    new_text[i] = '\0';

                    fr_set_memory (reg->reg_values[0], (void*) new_text);
                    fr_set_data_type (reg->reg_values[0], DT_STRING);
                }
                else if (data_type == DT_FLOAT && data_type_add != DT_FLOAT)
                    fr_set_memory (reg->reg_values[0], (void*)(intptr_t)((((intptr_t) register_list[m_value]->reg_values[0].value / FLOAT_CONV_VALUE) * m_value_mul) * FLOAT_CONV_VALUE));
                else if (data_type != DT_FLOAT && data_type_add == DT_FLOAT)
                    fr_set_memory (reg->reg_values[0], (void*)(intptr_t)((intptr_t) register_list[m_value]->reg_values[0].value * (m_value_mul / FLOAT_CONV_VALUE)));
                else if (data_type == DT_FLOAT && data_type_add == DT_FLOAT)
                    fr_set_memory (reg->reg_values[0], (void*)(intptr_t)((((intptr_t) register_list[m_value]->reg_values[0].value / FLOAT_CONV_VALUE) * ((float) m_value_mul / FLOAT_CONV_VALUE)) * FLOAT_CONV_VALUE));
                else
                    fr_set_memory (reg->reg_values[0], (void*) ((intptr_t) register_list[m_value]->reg_values[0].value * m_value_mul));
                continue;
            }
            case DIV:
            {
                intptr_t m_value       = (intptr_t) fr_get_memory (reg->reg_values[0]);
                intptr_t m_value_div   = (intptr_t) fr_get_memory (reg->reg_values[1]);
                byte     data_type_add = fr_get_data_type (reg->reg_values[1]);
                byte     data_type     = register_list[m_value]->reg_values[0].data_type;

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
                if (fr_get_memory (reg->reg_values[0]) == fr_get_memory (reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (reg->reg_values[2]) - 1;
                continue;
            }
            case BEQ:
            {
                if (fr_get_memory (reg->reg_values[0]) < fr_get_memory (reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (reg->reg_values[2]) - 1;
                continue;
            }
            case SEQ:
            {
                if (fr_get_memory (reg->reg_values[0]) > fr_get_memory (reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (reg->reg_values[2]) - 1;
                continue;
            }
            case BIG:
            {
                if (fr_get_memory (reg->reg_values[0]) <= fr_get_memory (reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (reg->reg_values[2]) - 1;
                continue;
            }
            case SMA:
            {
                if (fr_get_memory (reg->reg_values[0]) >= fr_get_memory (reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (reg->reg_values[2]) - 1;
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
//printf ("JUMP: %d\n", i);
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
