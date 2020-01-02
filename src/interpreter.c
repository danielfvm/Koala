#include "interpreter.h"
#include "parser.h"
#include "library.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define VOID(x) ((void*)(intptr_t)(x))

void kl_intp_register_create (Registry** register_list)
{
    (*register_list) = malloc (sizeof **register_list);
    (*register_list)[0] = NULL;
}

size_t kl_intp_register_add (Registry** register_list, Register* reg)
{
    size_t i;

    for (i = 0; (*register_list)[i]; ++ i);

    (*register_list) = realloc (*register_list, sizeof (Register) * (i + 1));    
    
    (*register_list)[i] = reg;
    (*register_list)[i + 1] = NULL;

    return i;
}

size_t kl_intp_get_current_register_position (Registry** register_list)
{
    size_t i;

    for (i = 0; (*register_list)[i]; ++ i);

    return i;
}

Value POINTER (int m_pointer)
{
    return (Value) {
        DT_POINTER,
        NO_SIZE,
        VOID (m_pointer)
    };
}

Value POINTER_POINTER (int m_pointer)
{
    return (Value) {
        DT_POINTER_POINTER,
        NO_SIZE,
        VOID (m_pointer)
    };
}

Value VALUE (byte data_type, void* value)
{ 
    return (Value) { 
        data_type, 
        NO_SIZE,
        value
    };
}

Value VALUE_STR (char* value)
{
    char* str;
    strcpy (str = malloc (strlen (value) + 1), value);

    return (Value) {
        DT_STRING, 
        NO_SIZE,
        str 
    };
}

Value VALUE_CHAR (char value)
{ 
    return (Value) { 
        DT_CHAR, 
        NO_SIZE,
        VOID (value)
    };
}

Value VALUE_INT (int value) 
{ 
    return (Value) { 
        DT_INT, 
        NO_SIZE,
        VOID (value)
    };
}

Value VALUE_FLOAT (float value)
{
    float* p_float = malloc (sizeof (float));
    *p_float = value;

    return (Value) { 
        DT_FLOAT, 
        NO_SIZE,
        p_float
    };
}

Value VALUE_NULL ()
{ 
    return (Value) { 
        DT_NONE, 
        NO_SIZE, 
        NULL 
    };
}

Value VALUE_LIST (Value* value, size_t size)
{ 
    return (Value) { 
        DT_LIST, 
        size,
        value 
    };
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
    return CREATE_REGISTER_3 (
        ALLOC, 
        VALUE_NULL (), 
        m_value, 
        VALUE_INT (false)
    );
}

Register* REGISTER_ADD (Value m_index, Value m_add) 
{ 
    return CREATE_REGISTER_2 (
        ADD, 
        m_index, 
        m_add
    ); 
}

Register* REGISTER_SUB (Value m_index, Value m_sub) 
{ 
    return CREATE_REGISTER_2 (
        SUB, 
        m_index, 
        m_sub
    );
}

Register* REGISTER_MUL (Value m_index, Value m_mul) 
{ 
    return CREATE_REGISTER_2 (
        MUL, 
        m_index, 
        m_mul
    );
}

Register* REGISTER_MOD (Value m_index, Value m_mul) 
{ 
    return CREATE_REGISTER_2 (
        MOD, 
        m_index, 
        m_mul
    );
}

Register* REGISTER_DIV (Value m_index, Value m_div)
{ 
    return CREATE_REGISTER_2 (
        DIV, 
        m_index, 
        m_div
    );
}

Register* REGISTER_READ (Value m_index) 
{ 
    return CREATE_REGISTER_1 (
        READ, 
        m_index
    );
}

Register* REGISTER_READ_CHAR (Value m_index) 
{ 
    return CREATE_REGISTER_1 (
        READ_CHAR, 
        m_index
    );
}

Register* REGISTER_SET (Value m_index, Value m_value) 
{ 
    return CREATE_REGISTER_2 (
        SET, 
        m_index, 
        m_value
    );
}

Register* REGISTER_SSET (Value m_index, Value m_value)
{
    return CREATE_REGISTER_2 (
        SSET,
        m_index,
        m_value
    );
}

Register* REGISTER_IND (Value m_value, Value m_index, Value m_value_index)
{
    return CREATE_REGISTER_3 (
        IND,
        m_value,
        m_index,
        m_value_index
    );
}

Register* REGISTER_IND_SET (Value m_value, Value m_index, Value m_set)
{
    return CREATE_REGISTER_3 (
        IND_SET,
        m_value,
        m_index,
        m_set
    );
}

Register* REGISTER_EQ (Value m_value1, Value m_value2, Value not_position)
{
    return CREATE_REGISTER_3 (
        EQ,
        m_value1,
        m_value2,
        not_position
    );
}

Register* REGISTER_NEQ (Value m_value1, Value m_value2, Value not_position)
{
    return CREATE_REGISTER_3 (
        NEQ,
        m_value1,
        m_value2,
        not_position
    );
}

Register* REGISTER_CMP (Value m_value1, Value m_value2, Value not_position)
{
    return CREATE_REGISTER_3 (
        CMP,
        m_value1,
        m_value2,
        not_position
    );
}

Register* REGISTER_NCMP (Value m_value1, Value m_value2, Value not_position)
{
    return CREATE_REGISTER_3 (
        NCMP,
        m_value1,
        m_value2,
        not_position
    );
}

Register* REGISTER_SYS (Value cmd)
{
    return CREATE_REGISTER_1 (
        SYS,
        cmd
    );
}

Register* REGISTER_OR (Value m_index, Value m_or) 
{ 
    return CREATE_REGISTER_2 (
        OR, 
        m_index, 
        m_or
    );
}

Register* REGISTER_AND (Value m_index, Value m_and) 
{
    return CREATE_REGISTER_2 (
        AND,
        m_index,
        m_and
    );
}

Register* REGISTER_POW (Value m_value1, Value m_value2, Value m_index)
{
    return CREATE_REGISTER_3 (
        POW,
        m_value1,
        m_value2,
        m_index
    );
}

Register* REGISTER_BIG (Value m_value1, Value m_value2, Value m_index)
{
    return CREATE_REGISTER_3 (
        BIG,
        m_value1,
        m_value2,
        m_index
    );
}

Register* REGISTER_SMA (Value m_value1, Value m_value2, Value m_index)
{
    return CREATE_REGISTER_3 (
        SMA,
        m_value1,
        m_value2,
        m_index
    );
}

Register* REGISTER_BEQ (Value m_value1, Value m_value2, Value m_index)
{
    return CREATE_REGISTER_3 (
        BEQ,
        m_value1,
        m_value2,
        m_index
    );
}

Register* REGISTER_SEQ (Value m_value1, Value m_value2, Value m_index)
{
    return CREATE_REGISTER_3 (
        SEQ,
        m_value1,
        m_value2,
        m_index
    );
}

Register* REGISTER_JUMP (Value position)
{
    return CREATE_REGISTER_1 (
        JUMP,
        position
    );
}

Register* REGISTER_PRINT (Value m_index)
{
    return CREATE_REGISTER_1 (
        PRINT,
        m_index
    );
}

Register* REGISTER_FLUSH (Value m_index)
{
    return CREATE_REGISTER_1 (
        FLUSH,
        m_index
    );
}

Register* REGISTER_NEG (Value m_index)
{
    return CREATE_REGISTER_1 (
        NEG,
        m_index
    );
}

Register* REGISTER_PUSH (Value m_index)
{
    return CREATE_REGISTER_1 (
        PUSH,
        m_index
    );
}

Register* REGISTER_POP (Value m_index)
{
    return CREATE_REGISTER_1 (
        POP,
        m_index
    );
}

Register* REGISTER_LIB_FUNC (Value id, Value ret, Value args)
{
    return CREATE_REGISTER_3 (
        LIB_FUNC,
        id,
        ret,
        args
    );
}

void kl_intp_strcpy (char** dest, const char* src)
{
    int dest_size = *dest == NULL ? 0 : strlen (*dest);
    int src_size  = strlen (src);
    int i;

    (*dest) = *dest == NULL ? malloc (src_size + 1) : realloc (*dest, dest_size + src_size + 1);

    for (i = dest_size; i < dest_size + src_size; ++ i)
        (*dest)[i] = (char)src[i - dest_size];
    (*dest)[i] = '\0';
}

Registry* register_list;

bool  kl_intp_set_pointer (size_t m_index, Value m_value)
{
    if (register_list[m_index]->reg_type != ALLOC)
        return false;
    register_list[m_index]->reg_values[0] = m_value;
    return true;
}

Value kl_intp_get_pointer (size_t m_index)
{
    return register_list[m_index]->reg_values[0];
}

Value kl_intp_get_memory_value (Value value)
{
    if (value.data_type == DT_POINTER || value.data_type == DT_POINTER_POINTER)
    {
        int max_size = kl_intp_get_current_register_position (&register_list) - 1; // TODO: Might not be -1 -> maybe too small
        if ((intptr_t) value.value < 0 || (intptr_t) value.value >= max_size)
            error ("Pointing on ´%p´ doesn't exist as Register!", (int) (intptr_t) value.value);
        if (register_list[(intptr_t) value.value]->reg_type != ALLOC)
            error ("Register ´%p´ isn't a Memory Register!", (int) (intptr_t) value.value);
            
        Value v1 = register_list[(intptr_t) value.value]->reg_values[0];
        if (value.data_type == DT_POINTER)
            return v1;

        int v2 = (int)(intptr_t) kl_intp_get_memory_value (v1).value;
        if (v2 < 0 || v2 >= max_size)
            error ("Pointer pointing on ´%p´ doesn't exist as Register!", v2);
        if (register_list[v2]->reg_type != ALLOC)
            error ("Register ´%p´ isn't a Memory Register!", (int) (intptr_t) value.value);

        return register_list[v2]->reg_values[0];
    }
    return value;
}

void* kl_intp_get_memory (Value value)
{
    return kl_intp_get_memory_value (value).value;
}

void kl_intp_set_memory (Value value, void* new_value)
{
    register_list[(intptr_t) kl_intp_get_memory (value)]->reg_values[0].value = new_value;
}

void kl_intp_set_size (Value value, size_t new_size)
{
    register_list[(intptr_t) kl_intp_get_memory (value)]->reg_values[0].size = new_size;
}

void kl_intp_set_memory_float (Value value, float new_value)
{
    (*(float*)register_list[(intptr_t) kl_intp_get_memory (value)]->reg_values[0].value) = new_value;
}


void kl_intp_set_data_type (Value value, int new_data_type)
{
    register_list[(intptr_t) kl_intp_get_memory (value)]->reg_values[0].data_type = new_data_type;
}

byte kl_intp_get_data_type (Value value)
{
    return kl_intp_get_memory_value (value).data_type;
}

size_t kl_intp_get_size (Value value)
{
    return kl_intp_get_memory_value (value).size;
}

#define kl_intp_get_as_number(value) ( \
    kl_intp_get_data_type (value) == DT_FLOAT ? *(float*)kl_intp_get_memory (value) : (intptr_t)kl_intp_get_memory (value) \
)

#define kl_intp_set_as_number(value, new_value) \
    if (kl_intp_get_data_type (register_list[(int)kl_intp_get_as_number (value)]->reg_values[0]) == DT_FLOAT) \
        kl_intp_set_memory_float (value, (float) (new_value)); \
    else \
        kl_intp_set_memory (value, VOID (new_value));

void print (Value value)
{
    byte m_type = kl_intp_get_data_type (value); 

    if (m_type == DT_STRING)
        printf ("%s", (char*) kl_intp_get_memory (value));
    else if (m_type == DT_INT)
        printf ("%d", (int) (intptr_t) kl_intp_get_memory (value));
    else if (m_type == DT_CHAR)
        printf ("%c",  (char) (intptr_t) kl_intp_get_memory (value));
    else if (m_type == DT_FLOAT)
        printf ("%f", *(float*)  kl_intp_get_memory (value));
    else if (m_type == DT_LIST)
    {
        printf ("[");

        size_t size = kl_intp_get_size (value);
        for (size_t i = 0; i < size; ++ i)
        {
            print (((Value*) kl_intp_get_memory (value))[i]);
            if (i != size - 1) printf (", ");
        }

        printf ("]");
    }
}

bool equ (Value v_one, Value v_two)
{
    if ((kl_intp_get_data_type (v_one) == DT_STRING && kl_intp_get_data_type (v_two) != DT_STRING) || 
        (kl_intp_get_data_type (v_two) == DT_STRING && kl_intp_get_data_type (v_one) != DT_STRING))
        return false;
    else if (kl_intp_get_data_type (v_one) == DT_STRING && kl_intp_get_data_type (v_two) == DT_STRING)
        return !strcmp (kl_intp_get_memory (v_one), kl_intp_get_memory (v_two));
    else
        return kl_intp_get_as_number (v_one) == kl_intp_get_as_number (v_two);
}

void alloc (Value* save, Value stored, Value sset)
{
    if (sset.value) // do not ALLOC if SET was called on this memory!
    {
        sset.value = false; // reset SALLOC
        return;
    }

    byte   def_data_type = kl_intp_get_data_type (stored);
    void*  def_value     = kl_intp_get_memory    (stored);
    size_t def_size      = kl_intp_get_size      (stored);

    if (def_data_type == DT_STRING)
    {
        size_t text_size = strlen (def_value) + 1;
        save->value = (save->value != NULL) ? realloc (save->value, text_size) : malloc (text_size);
        strcpy (save->value, def_value);
    }
    else if (def_data_type == DT_FLOAT)
    {
        if (save->value == NULL)
            save->value = malloc (sizeof (float)); 
        (*(float*)save->value) = (float)kl_intp_get_as_number (stored);
    }
    else if (def_data_type == DT_LIST)
    {
        if (save->value == NULL)
            save->value = malloc (sizeof (Value) * def_size); 
        else
            save->value = realloc (save->value, sizeof (Value) * def_size); 

        // TODO: Change ALLOC to func for recurs init of STRs, FLOATs & Lists
        for (size_t m_i = 0; m_i < def_size; ++ m_i)
        {
            Value value = ((Value*) def_value)[m_i];
            ((Value*) save->value)[m_i].value = NULL;
            alloc (&((Value*) save->value)[m_i], value, VALUE_INT (false));
            //((Value*) save->value)[m_i].data_type = kl_intp_get_data_type (value);
            //((Value*) save->value)[m_i].value     = kl_intp_get_memory    (value);
            //((Value*) save->value)[m_i].size      = kl_intp_get_size      (value);
        }
    }
    else
    {
        save->value = kl_intp_get_memory (stored);
    }

    save->data_type = def_data_type;
    save->size = def_size;
}

int kl_intp_run (Registry* _register_list)
{
    register_list = _register_list;

    if (!register_list)
    {
        fprintf (stderr, "´register_list´ is NULL!\n");
        return EXIT_FAILURE;
    }

    int size;

    // Calculating size of ´register_list´
    for (size = 0; register_list[size]; ++ size);

    // Stack
    Memory* stack = malloc (sizeof (Memory));
    size_t  stack_size = 0;

    // Start interpreting
    for (int i = 0; i < size && i >= 0; ++ i)
    {
        Register* reg = register_list[i];

        switch (reg->reg_type)
        {
            case PUSH:
            {
                stack[stack_size] = kl_intp_get_memory_value (reg->reg_values[0]);
                stack = realloc (stack, sizeof (Memory) * ((++ stack_size) + 1));
                continue;
            }
            case POP:
            {
                if (stack_size == 0)
                    continue;
                stack = realloc (stack, sizeof (Memory) * (stack_size --));
                register_list[(intptr_t)kl_intp_get_memory (reg->reg_values[0])]->reg_values[0] = stack[stack_size];
                continue;
            }
            case ALLOC:
            {/*
                if (reg->reg_values[2].value) // do not ALLOC if SET was called on this memory!
                {
                    reg->reg_values[2].value = false; // reset SALLOC
                    continue;
                }

                if (kl_intp_get_data_type (reg->reg_values[1]) == DT_STRING)
                {
                    size_t text_size = strlen (kl_intp_get_memory (reg->reg_values[1])) + 1;
                    void** ref_value = &reg->reg_values[0].value;
                    *ref_value = (*ref_value != NULL) ? realloc (*ref_value, text_size) : malloc (text_size);
                    strcpy (reg->reg_values[0].value, kl_intp_get_memory (reg->reg_values[1]));
                    reg->reg_values[0].data_type = DT_STRING;
                }
                else if (kl_intp_get_data_type (reg->reg_values[1]) == DT_FLOAT)
                {
                    if (reg->reg_values[0].value == NULL)
                        reg->reg_values[0].value = malloc (sizeof (float)); 
                    (*(float*)reg->reg_values[0].value) = (float)kl_intp_get_as_number (reg->reg_values[1]);
                    reg->reg_values[0].data_type = DT_FLOAT;
                }
                else if (kl_intp_get_data_type (reg->reg_values[1]) == DT_LIST)
                {
                    size_t list_size = kl_intp_get_size (reg->reg_values[1]);
                    if (reg->reg_values[0].value == NULL)
                        reg->reg_values[0].value = malloc (sizeof (Value) * list_size); 
//                    else
  //                      reg->reg_values[0].value = realloc (reg->reg_values[0].value, sizeof (Value) * list_size); 

                    // TODO: Change ALLOC to func for recurs init of STRs, FLOATs & Lists
                    for (size_t m_i = 0; m_i < list_size; ++ m_i)
                    {
                        Value value = ((Value*) kl_intp_get_memory(reg->reg_values[1]))[m_i];
                        ((Value*)reg->reg_values[0].value)[m_i].data_type = kl_intp_get_data_type (value);
                        ((Value*)reg->reg_values[0].value)[m_i].value = kl_intp_get_memory (value);
                        ((Value*)reg->reg_values[0].value)[m_i].size = kl_intp_get_size (value);
                    }

                    reg->reg_values[0].data_type = DT_LIST;
                    reg->reg_values[0].size = list_size;
                }
                else
                {
                    reg->reg_values[0].value = kl_intp_get_memory (reg->reg_values[1]);
                    reg->reg_values[0].data_type = kl_intp_get_data_type (reg->reg_values[1]);
                }*/

                alloc (&reg->reg_values[0], reg->reg_values[1], reg->reg_values[2]);
                continue;
            }
            case IND:
            {
                if (kl_intp_get_data_type (reg->reg_values[2]) != DT_INT)
                    error ("Expected DataType ´INT´ for indexing", NULL);

                // Get value from array using this ´index´
                int index = (intptr_t)kl_intp_get_memory (reg->reg_values[2]);

                if (kl_intp_get_data_type (reg->reg_values[1]) == DT_STRING)
                {
                    char* str = kl_intp_get_memory (reg->reg_values[1]);

                    if (index < 0 || index > (int) strlen (str))
                        error ("Index ´%d´ out of Stringbounds[%d]", VOID (index), VOID (strlen (str)));

                    kl_intp_set_data_type (reg->reg_values[0], DT_CHAR);
                    kl_intp_set_memory    (reg->reg_values[0], VOID (str[index]));
                }
                else if (kl_intp_get_data_type (reg->reg_values[1]) == DT_LIST)
                {
                    int size = kl_intp_get_size (reg->reg_values[1]);

                    if (index < 0 || index >= size)
                        error ("Index ´%d´ out of Arraybounds[%d]", VOID (index), VOID (size));

                    Value value = ((Value*)kl_intp_get_memory (reg->reg_values[1]))[index];
                    kl_intp_set_data_type (reg->reg_values[0], value.data_type);
                    kl_intp_set_memory (reg->reg_values[0], value.value);
                    kl_intp_set_size (reg->reg_values[0], value.size);
                }
                else
                    error ("DataType does not support indexing", NULL);
                continue;
            }
            case IND_SET:
            {
                if (kl_intp_get_data_type (reg->reg_values[1]) != DT_INT)
                    error ("Expected DataType ´INT´ for indexing", NULL);
                if (kl_intp_get_data_type (reg->reg_values[0]) == DT_STRING)
                {
                    char* value = &((char*)kl_intp_get_memory (reg->reg_values[0]))[(intptr_t) kl_intp_get_memory (reg->reg_values[1])];
                    *value = (intptr_t) kl_intp_get_memory (reg->reg_values[2]);
                }
                else if (kl_intp_get_data_type (reg->reg_values[0]) == DT_LIST)
                {
                    alloc (
                        &((Value*)kl_intp_get_memory (reg->reg_values[0]))[(intptr_t) kl_intp_get_memory (reg->reg_values[1])], 
                        reg->reg_values[2],
                        VALUE_INT (false)
                    ); 
                }
                else
                    error ("DataType does not support indexing", NULL);
                continue;
            }
            case SSET:
            {
                register_list[(intptr_t)kl_intp_get_memory (reg->reg_values[0])]->reg_values[2].value = VOID (true); // used in ALLOC
                __attribute__ ((fallthrough));
            case SET:
                // TODO: FIX ALLOC BUG
                /*alloc (
                    &reg->reg_values[0],
                    reg->reg_values[1],
                    VALUE_INT (false)
                );*/ 
                kl_intp_set_data_type (reg->reg_values[0], kl_intp_get_data_type (reg->reg_values[1]));
                kl_intp_set_memory    (reg->reg_values[0], kl_intp_get_memory    (reg->reg_values[1]));
                kl_intp_set_size      (reg->reg_values[0], kl_intp_get_size      (reg->reg_values[1]));
                continue;
            }
            case NEG:
            {
                kl_intp_set_memory (
                    reg->reg_values[0], 
                    VOID (kl_intp_get_memory (POINTER ((intptr_t)kl_intp_get_memory (reg->reg_values[0]))) ? 0 : 1)
                );
                continue;
            }
            case ADD:
            {
                intptr_t m_value     = (intptr_t) kl_intp_get_memory (reg->reg_values[0]);
                intptr_t m_value_add = (intptr_t) kl_intp_get_memory (reg->reg_values[1]);

                if (register_list[m_value]->reg_values[0].data_type == DT_STRING)
                {
                    char* value    = register_list[m_value]->reg_values[0].value;
                    byte data_type = kl_intp_get_data_type (reg->reg_values[1]);

                    void** m_value = &register_list[(intptr_t) kl_intp_get_memory (reg->reg_values[0])]->reg_values[0].value;

                    if (data_type == DT_STRING)
                    {
                        char* value_add = kl_intp_get_memory (reg->reg_values[1]);
                        (*m_value) = realloc (*m_value, strlen (value) + strlen (value_add) + 1);
                        strcat (*m_value, value_add); 
                    }
                    else if (data_type == DT_CHAR)
                    {
                        int size = strlen (*m_value);
                        (*m_value) = realloc (*m_value, strlen (*m_value) + 2);
                        ((char*) *m_value)[size] = m_value_add;
                        ((char*) *m_value)[size + 1] = '\0';
                    }
                    else if (data_type == DT_FLOAT || data_type == DT_INT)
                    {
                        char* str_num = malloc (38 + 1);

                        if (data_type == DT_FLOAT)
                            sprintf (str_num, "%f", kl_intp_get_as_number (reg->reg_values[1]));
                        else
                            sprintf (str_num, "%d", (int)kl_intp_get_as_number (reg->reg_values[1]));

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
                else if (register_list[m_value]->reg_values[0].data_type == DT_CHAR && kl_intp_get_data_type (reg->reg_values[1]) == DT_STRING)
                {
                    void** m_value = &register_list[(intptr_t) kl_intp_get_memory (reg->reg_values[0])]->reg_values[0].value;
                    register_list[(intptr_t) kl_intp_get_memory (reg->reg_values[0])]->reg_values[0].data_type = DT_STRING;
                    int size = strlen ((char*) m_value_add);
                    char c = (intptr_t)(*m_value);
                    (*m_value) = (char*) malloc (size + 2);
                    sprintf (*m_value, "%c%s", c, (char*) m_value_add);
                    ((char*) *m_value)[size + 1] = '\0';
                    continue;
                }
                else if (register_list[m_value]->reg_values[0].data_type == DT_LIST)
                {
                    Value* v_list = &register_list[m_value]->reg_values[0];
                    v_list->size += 1;
                    v_list->value = realloc (v_list->value, v_list->size);
                    alloc (
                        &((Value*)v_list->value)[v_list->size - 1],
                        reg->reg_values[1],
                        VALUE_INT (false)
                    );
//                    ((Value*)v_list->value)[v_list->size - 1].data_type = kl_intp_get_data_type (reg->reg_values[1]); // TODO: Copy 
//                    ((Value*)v_list->value)[v_list->size - 1].value     = kl_intp_get_memory    (reg->reg_values[1]); // TODO: Copy 
//                    ((Value*)v_list->value)[v_list->size - 1].size      = kl_intp_get_size      (reg->reg_values[1]); // TODO: Copy 
                }
                else
                {
                    kl_intp_set_as_number (
                        reg->reg_values[0], 
                        kl_intp_get_as_number (register_list[(intptr_t)kl_intp_get_as_number (reg->reg_values[0])]->reg_values[0]) + 
                            (float)kl_intp_get_as_number (reg->reg_values[1])
                    );
                }
                continue;
            }
            case SUB:
            {
                kl_intp_set_as_number (
                    reg->reg_values[0], 
                    kl_intp_get_as_number (register_list[(intptr_t)kl_intp_get_as_number (reg->reg_values[0])]->reg_values[0]) - 
                        (float)kl_intp_get_as_number (reg->reg_values[1])
                );
                continue;
            }
            case MUL:
            {
                void** value = &register_list[(intptr_t) kl_intp_get_memory (reg->reg_values[0])]->reg_values[0].value;
                intptr_t m_value_mul = (intptr_t) kl_intp_get_memory (reg->reg_values[1]);
                byte data_type_add = kl_intp_get_data_type (reg->reg_values[1]);
                byte data_type     = register_list[(intptr_t) kl_intp_get_memory (reg->reg_values[0])]->reg_values[0].data_type;

                size_t i; 

                if ((data_type == DT_STRING && data_type_add == DT_INT) || (data_type == DT_INT && data_type_add == DT_STRING))
                {
                    intptr_t text_mul = (data_type == DT_STRING) ? (intptr_t) m_value_mul : (intptr_t)(*value);
                    text_mul = (int) text_mul < 0 ? 0 : text_mul;

                    char* text_copy = NULL;
                    kl_intp_strcpy (&text_copy, (data_type != DT_STRING) ? (char*) m_value_mul : (char*)(*value));

                    if (data_type == DT_STRING)
                        free (*value);
                    (*value) = NULL;

                    for (; text_mul > 0; -- text_mul)
                        kl_intp_strcpy ((char**) value, text_copy);
                    free (text_copy);

                    kl_intp_set_data_type (reg->reg_values[0], DT_STRING);
                }
                else if ((data_type == DT_CHAR && data_type_add == DT_INT) || (data_type == DT_INT && data_type_add == DT_CHAR))
                {
                    intptr_t new_text_size = (data_type == DT_CHAR) ? m_value_mul : (intptr_t)(*value);
                    new_text_size = (int) new_text_size < 0 ? 0 : new_text_size;

                    char char_copy = (data_type != DT_CHAR) ? m_value_mul : (intptr_t)(*value);

                    (*value) = malloc (new_text_size + 1);
                    for (i = 0; i < (size_t)new_text_size; ++ i)
                        ((char*)(*value))[i] = char_copy;
                    ((char*)(*value))[i] = '\0';

                    kl_intp_set_data_type (reg->reg_values[0], DT_STRING);
                }
                else
                {
                    kl_intp_set_as_number (
                        reg->reg_values[0], 
                        kl_intp_get_as_number (register_list[(intptr_t)kl_intp_get_as_number (reg->reg_values[0])]->reg_values[0]) * 
                            (float) kl_intp_get_as_number (reg->reg_values[1])
                    );
                }
                continue;
            }
            case MOD:
            {
                kl_intp_set_as_number (
                    reg->reg_values[0], 
                    (int) kl_intp_get_as_number (register_list[(intptr_t)kl_intp_get_as_number (reg->reg_values[0])]->reg_values[0]) % 
                        (int) kl_intp_get_as_number (reg->reg_values[1])
                );
                continue;
            }
            case DIV:
            {
                kl_intp_set_as_number (
                    reg->reg_values[0], 
                    kl_intp_get_as_number (register_list[(intptr_t)kl_intp_get_as_number (reg->reg_values[0])]->reg_values[0]) / 
                        (float) kl_intp_get_as_number (reg->reg_values[1])
                );
                continue;
            }
            case CMP:
            {
                if (kl_intp_get_as_number (reg->reg_values[0]) != kl_intp_get_as_number (reg->reg_values[1]))
                    i = (intptr_t) kl_intp_get_memory (reg->reg_values[2]) - 1;
                continue;
            }
            case NCMP:
            {
                if (kl_intp_get_as_number (reg->reg_values[0]) == kl_intp_get_as_number (reg->reg_values[1]))
                    i = (intptr_t) kl_intp_get_memory (reg->reg_values[2]) - 1;
                continue;
            }
            case BEQ:
            {
                kl_intp_set_memory (
                    reg->reg_values[2], 
                    VOID (kl_intp_get_as_number (reg->reg_values[0]) >= kl_intp_get_as_number (reg->reg_values[1]))
                );
                continue;
            }
            case SEQ:
            {
                kl_intp_set_memory (
                    reg->reg_values[2], 
                    VOID (kl_intp_get_as_number (reg->reg_values[0]) <= kl_intp_get_as_number (reg->reg_values[1]))
                );
                continue;
            }
            case POW:
            {
                kl_intp_set_as_number (
                    reg->reg_values[2], 
                    pow (kl_intp_get_as_number (reg->reg_values[0]), kl_intp_get_as_number (reg->reg_values[1]))
                );
                continue;
            }
            case BIG:
            {
                kl_intp_set_memory (
                    reg->reg_values[2], 
                    VOID (kl_intp_get_as_number (reg->reg_values[0]) > kl_intp_get_as_number (reg->reg_values[1]))
                );
                continue;
            }
            case OR:
            {
                intptr_t m_value     = (intptr_t) kl_intp_get_as_number (reg->reg_values[0]);
                intptr_t m_value_sub = (intptr_t) kl_intp_get_as_number (reg->reg_values[1]);
                kl_intp_set_memory (
                    reg->reg_values[0], 
                    VOID (kl_intp_get_as_number (register_list[m_value]->reg_values[0]) || m_value_sub)
                );
                continue;
            }
            case AND:
            {
                intptr_t m_value     = (intptr_t) kl_intp_get_as_number (reg->reg_values[0]);
                intptr_t m_value_sub = (intptr_t) kl_intp_get_as_number (reg->reg_values[1]);
                kl_intp_set_memory (reg->reg_values[0], VOID (kl_intp_get_as_number (register_list[m_value]->reg_values[0]) && m_value_sub));
                continue;
            }
            case SMA:
            {
                kl_intp_set_memory (
                    reg->reg_values[2], 
                    VOID (kl_intp_get_as_number (reg->reg_values[0]) < kl_intp_get_as_number (reg->reg_values[1]))
                );
                continue;
            }
            case EQ:
            {
                kl_intp_set_memory (reg->reg_values[2], VOID (equ (reg->reg_values[0], reg->reg_values[1])));
                continue;
            }
            case NEQ:
            {
                kl_intp_set_memory (reg->reg_values[2], VOID (!equ (reg->reg_values[0], reg->reg_values[1])));
                continue;
            }
            case PRINT:
            {
                print (reg->reg_values[0]);
                continue;
            }
            case FLUSH:
            {
                print (reg->reg_values[0]);
                fflush (NULL);
                continue;
            }
            case READ:
            {
                byte   m_type  =  register_list[(intptr_t) kl_intp_get_memory (reg->reg_values[0])]->reg_values[0].data_type; 
                void** m_value = &register_list[(intptr_t) kl_intp_get_memory (reg->reg_values[0])]->reg_values[0].value;

                if (m_type == DT_STRING)
                {
                    size_t buffer_size = 100000;
                    char*  buffer = malloc (buffer_size);

                    fgets (buffer, buffer_size, stdin);

                    *m_value = realloc (*m_value, strlen (buffer) + 1);
                    strcpy (*m_value, buffer);
                    free (buffer);
                }
                else if (m_type == DT_INT)
                {
                    scanf ("%d", (int*)m_value);
                    getchar();
                }
                else if (m_type == DT_CHAR)
                {
                    scanf ("%c", (char*)m_value);
                    getchar();
                }
                else if (m_type == DT_FLOAT)
                {
                    float value;
                    scanf ("%f", &value);
                    *(float*)(*m_value) = value;
                    getchar();
                }
                continue;
            }
            case READ_CHAR:
            {
                #ifdef __linux__
                system ("/bin/stty raw");
                #endif

                register_list[(intptr_t) kl_intp_get_memory (reg->reg_values[0])]->reg_values[0].data_type = DT_CHAR; 
                register_list[(intptr_t) kl_intp_get_memory (reg->reg_values[0])]->reg_values[0].value = VOID (getchar());

                #ifdef __linux__
                system ("/bin/stty cooked");
                #endif
                continue;
            }
            case JUMP:
            {
                i = (intptr_t) kl_intp_get_memory (reg->reg_values[0]) - 1;
                continue;
            }
            case SYS:
            {
                system ((char*) kl_intp_get_memory (reg->reg_values[0]));
                continue;
            }
            case LIB_FUNC:
            {
                int   lib_func_id = kl_intp_get_as_number (reg->reg_values[0]);
                size_t args_count = kl_intp_get_size      (reg->reg_values[2]);
                Value* args_value = kl_intp_get_memory    (reg->reg_values[2]);
                Value* init_args_value = malloc (sizeof (Value) * args_count);

                // Dereference Pointers & get their Values
                for (size_t args_i = 0; args_i < args_count; ++ args_i)
                    init_args_value[args_i] = kl_intp_get_memory_value (args_value[args_i]);

                // Call STD-Lib-Function
                register_list[(intptr_t) kl_intp_get_memory (reg->reg_values[1])]->reg_values[0] = kl_lib_exec_function (
                    lib_func_id,        // Function-Lib-ID
                    args_count,         // Arguments Count
                    init_args_value     // Arguments Value
                );

                // Free alloc memory
                free (init_args_value);

                continue;
            }
        }
    }

    // TODO: free memory of Value STRING, LIST, FLOAT.
    for (size_t i = 0; register_list[i]; ++ i)
    {
        free (register_list[i]->reg_values);
        free (register_list[i]);
    }

    free (stack);

    return EXIT_SUCCESS;
}
