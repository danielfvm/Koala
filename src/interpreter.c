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
    return (Value) { DT_POINTER, VALUE_POINTER, (void*)(intptr_t) m_pointer };
}

Value INDEX (int m_pointer, int index)
{
    return (Value) { DT_POINTER, index, (void*)(intptr_t) m_pointer };
}

Value VALUE (byte data_type, void* value)
{
    return (Value) { data_type, VALUE_VALUE, value };
}

Value VALUE_STR (void* value)
{
    return (Value) { DT_STRING, VALUE_VALUE, value };
}

Value VALUE_CHAR (char value)
{
    return (Value) { DT_CHAR, VALUE_VALUE, (void*)(intptr_t) value };
}

Value VALUE_INT (int value)
{
    return (Value) { DT_INT, VALUE_VALUE, (void*)(intptr_t) value };
}

Value VALUE_FLOAT (float value)
{
    return (Value) { DT_FLOAT, VALUE_VALUE, (void*)(long long)(value * FLOAT_CONV_VALUE) };
}

Register* REGISTER_ALLOC (Value m_value)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type   = ALLOC;
    reg->reg_values[0] = m_value;
    reg->mem_data_type = m_value.data_type;
    reg->mem_value = NULL;
    return reg;
}

Register* REGISTER_ADD (Value m_index, Value m_add)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = ADD;
    reg->reg_values[0] = m_index;
    reg->reg_values[1] = m_add;
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
    return reg;
}

Register* REGISTER_SUB (Value m_index, Value m_sub)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = SUB;
    reg->reg_values[0] = m_index;
    reg->reg_values[1] = m_sub;
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
    return reg;
}

Register* REGISTER_MUL (Value m_index, Value m_mul)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = MUL;
    reg->reg_values[0] = m_index;
    reg->reg_values[1] = m_mul;
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
    return reg;
}

Register* REGISTER_CIN (Value m_index)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type   = CIN;
    reg->reg_values[0] = m_index;
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
    return reg;
}

Register* REGISTER_DIV (Value m_index, Value m_div)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = DIV;
    reg->reg_values[0] = m_index;
    reg->reg_values[1] = m_div;
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
    return reg;
}

Register* REGISTER_SET (Value m_index, Value m_value)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values * 2);
    reg->reg_type   = SET;
    reg->reg_values[0] = m_index;
    reg->reg_values[1] = m_value;
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
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
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
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
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
    return reg;
}

Register* REGISTER_SYS (Value cmd)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type   = SYS;
    reg->reg_values[0] = cmd;
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
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
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
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
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
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
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
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
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
    return reg;
}

Register* REGISTER_FREE (Value m_index)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type   = FREE;
    reg->reg_values[0] = m_index;
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
    return reg;
}

Register* REGISTER_JMP (Value position)
{
    Register* reg   = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type   = JUMP;
    reg->reg_values[0] = position;
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
    return reg;
}

Register* REGISTER_OUT (Value m_index)
{
    Register* reg = malloc (sizeof *reg);
    reg->reg_values = malloc (sizeof *reg->reg_values);
    reg->reg_type = OUT;
    reg->reg_values[0] = m_index;
    reg->mem_data_type = DT_NONE;
    reg->mem_value = NULL;
    return reg;
}

void* fr_get_memory (const Registry* register_list, Value value)
{
    if (value.index == VALUE_VALUE)
        return value.value;
    return register_list[(intptr_t) value.value]->mem_value;
}

void fr_set_memory (const Registry* register_list, Value* value, void* new_value)
{
    register_list[(intptr_t) fr_get_memory (register_list, *value)]->mem_value = new_value;
}

byte fr_get_data_type (const Registry* register_list, Value value)
{
    if (value.index == VALUE_VALUE)
        return value.data_type;
    return register_list[(intptr_t) value.value]->mem_data_type;
}

int fr_run (const Registry* register_list)
{
    if (!register_list)
    {
        fprintf (stderr, "´register_list´ is NULL!\n");
        return EXIT_FAILURE;
    }

    size_t size;

    // Calculating size of ´register_list´
    for (size = 0; register_list[size]; ++ size);

    // Start interpreting
    for (size_t i = 0; i < size; ++ i)
    {
        Register* reg = register_list[i];

        switch (reg->reg_type)
        {
            case ALLOC:
            {
                reg->mem_value = fr_get_memory (register_list, reg->reg_values[0]);
                reg->mem_data_type = fr_get_data_type (register_list, reg->reg_values[0]);
                continue;
            }
            case SET:
            {
                intptr_t m_value     = (intptr_t) fr_get_memory (register_list, reg->reg_values[0]);
                intptr_t m_value_set = (intptr_t) fr_get_memory (register_list, reg->reg_values[1]);
                register_list[(intptr_t) fr_get_memory (register_list, reg->reg_values[0])]->mem_value     = (void*) m_value_set;
                register_list[(intptr_t) fr_get_memory (register_list, reg->reg_values[0])]->mem_data_type = fr_get_data_type (register_list, reg->reg_values[1]);
                continue;
            }
            case ADD:
            {
                intptr_t m_value     = (intptr_t) fr_get_memory (register_list, reg->reg_values[0]);
                intptr_t m_value_add = (intptr_t) fr_get_memory (register_list, reg->reg_values[1]);

                if (register_list[m_value]->mem_data_type == DT_FLOAT && fr_get_data_type (register_list, reg->reg_values[1]) != DT_FLOAT)
                    m_value_add *= FLOAT_CONV_VALUE;
                else if (register_list[m_value]->mem_data_type != DT_FLOAT && fr_get_data_type (register_list, reg->reg_values[1]) == DT_FLOAT)
                    m_value_add /= FLOAT_CONV_VALUE;

                fr_set_memory (register_list, &reg->reg_values[0], (void*) (register_list[m_value]->mem_value + m_value_add));
                continue;
            }
            case SUB:
            {
                intptr_t m_value     = (intptr_t) fr_get_memory (register_list, reg->reg_values[0]);
                intptr_t m_value_sub = (intptr_t) fr_get_memory (register_list, reg->reg_values[1]);

                if (register_list[m_value]->mem_data_type == DT_FLOAT && fr_get_data_type (register_list, reg->reg_values[1]) != DT_FLOAT)
                    m_value_sub *= FLOAT_CONV_VALUE;
                else if (register_list[m_value]->mem_data_type != DT_FLOAT && fr_get_data_type (register_list, reg->reg_values[1]) == DT_FLOAT)
                    m_value_sub /= FLOAT_CONV_VALUE;

                fr_set_memory (register_list, &reg->reg_values[0], (void*) (register_list[m_value]->mem_value - m_value_sub));
                continue;
            }
            case MUL:
            {
                intptr_t m_value     = (intptr_t) fr_get_memory (register_list, reg->reg_values[0]);
                intptr_t m_value_mul = (intptr_t) fr_get_memory (register_list, reg->reg_values[1]);
                byte     data_type_add = fr_get_data_type (register_list, reg->reg_values[1]);
                byte     data_type     = register_list[m_value]->mem_data_type;

                if (data_type == DT_FLOAT && data_type_add != DT_FLOAT)
                    fr_set_memory (register_list, &reg->reg_values[0], (void*)(intptr_t)((((intptr_t)register_list[m_value]->mem_value / FLOAT_CONV_VALUE) * m_value_mul) * FLOAT_CONV_VALUE));
                else if (data_type != DT_FLOAT && data_type_add == DT_FLOAT)
                    fr_set_memory (register_list, &reg->reg_values[0], (void*)(intptr_t)((intptr_t)register_list[m_value]->mem_value * (m_value_mul / FLOAT_CONV_VALUE)));
                else if (data_type == DT_FLOAT && data_type_add == DT_FLOAT)
                    fr_set_memory (register_list, &reg->reg_values[0], (void*)(intptr_t)((((intptr_t)register_list[m_value]->mem_value / FLOAT_CONV_VALUE) * ((float)m_value_mul / FLOAT_CONV_VALUE)) * FLOAT_CONV_VALUE));
                else
                    fr_set_memory (register_list, &reg->reg_values[0], (void*) ((intptr_t)register_list[m_value]->mem_value * m_value_mul));
                continue;
            }
            case DIV:
            {
                intptr_t m_value       = (intptr_t) fr_get_memory (register_list, reg->reg_values[0]);
                intptr_t m_value_div   = (intptr_t) fr_get_memory (register_list, reg->reg_values[1]);
                byte     data_type_add = fr_get_data_type (register_list, reg->reg_values[1]);
                byte     data_type     = register_list[m_value]->mem_data_type;

                if (data_type == DT_FLOAT && data_type_add != DT_FLOAT) 
                    fr_set_memory (register_list, &reg->reg_values[0], (void*)(intptr_t)((((intptr_t)register_list[m_value]->mem_value / FLOAT_CONV_VALUE) / m_value_div) * FLOAT_CONV_VALUE));
                else if (data_type != DT_FLOAT && data_type_add == DT_FLOAT)
                    fr_set_memory (register_list, &reg->reg_values[0], (void*)(intptr_t)((intptr_t)register_list[m_value]->mem_value / (m_value_div / FLOAT_CONV_VALUE)));
                else if (data_type == DT_FLOAT && data_type_add == DT_FLOAT)
                    fr_set_memory (register_list, &reg->reg_values[0], (void*)(intptr_t)((((intptr_t)register_list[m_value]->mem_value / FLOAT_CONV_VALUE) / ((float)m_value_div / FLOAT_CONV_VALUE)) * FLOAT_CONV_VALUE));
                else
                    fr_set_memory (register_list, &reg->reg_values[0], (void*) ((intptr_t)register_list[m_value]->mem_value / m_value_div));
                continue;
            }
            case EQ:
            {
                if (fr_get_memory (register_list, reg->reg_values[0]) != fr_get_memory (register_list, reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (register_list, reg->reg_values[2]) - 1;
                continue;
            }
            case NEQ:
            {
                if (fr_get_memory (register_list, reg->reg_values[0]) == fr_get_memory (register_list, reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (register_list, reg->reg_values[2]) - 1;
                continue;
            }
            case BEQ:
            {
                if (fr_get_memory (register_list, reg->reg_values[0]) < fr_get_memory (register_list, reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (register_list, reg->reg_values[2]) - 1;
                continue;
            }
            case SEQ:
            {
                if (fr_get_memory (register_list, reg->reg_values[0]) > fr_get_memory (register_list, reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (register_list, reg->reg_values[2]) - 1;
                continue;
            }
            case BIG:
            {
                if (fr_get_memory (register_list, reg->reg_values[0]) <= fr_get_memory (register_list, reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (register_list, reg->reg_values[2]) - 1;
                continue;
            }
            case SMA:
            {
                if (fr_get_memory (register_list, reg->reg_values[0]) >= fr_get_memory (register_list, reg->reg_values[1]))
                    i = (intptr_t) fr_get_memory (register_list, reg->reg_values[2]) - 1;
                continue;
            }
            case OUT:
            {
                byte m_type = fr_get_data_type (register_list, reg->reg_values[0]); 
                if (m_type == DT_STRING)
                    printf ("%s", (char*) fr_get_memory (register_list, reg->reg_values[0]));
                else if (m_type == DT_INT || m_type == DT_BOOL)
                    printf ("%d", (intptr_t) fr_get_memory (register_list, reg->reg_values[0]));
                else if (m_type == DT_CHAR)
                    printf ("%c", (intptr_t) fr_get_memory (register_list, reg->reg_values[0]));
                else if (m_type == DT_FLOAT)
                    printf ("%f", (intptr_t) fr_get_memory (register_list, reg->reg_values[0]) / FLOAT_CONV_VALUE);
                continue;
            }
            case CIN:
            {
                byte   m_type  = register_list[(intptr_t) fr_get_memory (register_list, reg->reg_values[0])]->mem_data_type; 
                void** m_value = &register_list[(intptr_t) fr_get_memory (register_list, reg->reg_values[0])]->mem_value;

                if (m_type == DT_STRING)
                {
                    size_t buffer_size = 100000;
                    char*  buffer = malloc (sizeof (char) * buffer_size);

                    getline (&buffer, &buffer_size, stdin);

                    buffer[strlen (buffer) - 1] = '\0';
                    *m_value = buffer;

                    free (buffer);
                }
                else if (m_type == DT_INT || m_type == DT_BOOL)
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
                i = (intptr_t) fr_get_memory (register_list, reg->reg_values[0]) - 1;
                continue;
            }
            case FREE:
            {
                register_list[(intptr_t) fr_get_memory (register_list, reg->reg_values[0])]->mem_value     = NULL;
                register_list[(intptr_t) fr_get_memory (register_list, reg->reg_values[0])]->mem_data_type = NONE;
                continue;
            }
            case SYS:
            {
                system ((char*) fr_get_memory (register_list, reg->reg_values[0]));
                continue;
            }
        }
    }

    for (size_t i = 0; register_list[i]; ++ i)
    {
        free (register_list[i]->reg_values);
        free (register_list[i]);
    }

    return EXIT_SUCCESS;
}
