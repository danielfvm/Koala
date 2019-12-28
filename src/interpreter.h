#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdlib.h>

#define FLOAT_CONV_VALUE 1000000.0

typedef unsigned char byte;

enum Execution
{
    NONE,
    ALLOC,
    SALLOC,
    PRINT,
    READ,
    EQ,
    NEQ,
    BIG,
    SMA,
    BEQ,
    SEQ,
    JUMP,
    ADD,
    SUB,
    MUL,
    MOD,
    DIV,
    SET,
    SYS,
    NEG,
    PUSH,
    POP,
    IND,
    CMP,
    NCMP,
    READ_CHAR,
    CODE,
    FLUSH
};

enum DataType
{
    DT_NONE,
    DT_POINTER,
    DT_POINTER_POINTER,
    DT_INT,
    DT_FLOAT,
    DT_CHAR,
    DT_STRING
};

enum ValueType
{
    VALUE_TYPE_VALUE   = -3,
    VALUE_TYPE_POINTER = -2,
    VALUE_TYPE_POINTER_POINTER = -1,
};


typedef struct
{
    byte  data_type;
    int   index;
    void* value;
} Value;

typedef struct
{
    byte   reg_type;
    Value* reg_values;
} Register;

typedef Register* Registry;

typedef Value Memory;

void fr_register_create (Registry** register_list);

size_t fr_register_add (Registry** register_list, Register* reg);

size_t fr_register_add_all (Registry** register_list, Register** regs);

size_t fr_get_current_register_position (Registry** register_list);

char* fr_get_register_type_as_name (Register* reg);

Value POINTER (int m_pointer);

Value POINTER_POINTER (int m_pointer);

Value VALUE (byte data_type, void* value);

Value VALUE_STR (void* value);

Value VALUE_CHAR (char value);

Value VALUE_INT (int value);

Value VALUE_FLOAT (float value);

Value VALUE_VALUE (Value value);


Register* REGISTER_ALLOC (Value m_value);

Register* REGISTER_SALLOC (Value m_value);

Register* REGISTER_ADD (Value m_index, Value m_add);

Register* REGISTER_SUB (Value m_index, Value m_sub);

Register* REGISTER_MUL (Value m_index, Value m_mul);

Register* REGISTER_MOD (Value m_index, Value m_mul);

Register* REGISTER_READ (Value m_index);

Register* REGISTER_READ_CHAR (Value m_index);

Register* REGISTER_DIV (Value m_index, Value m_div);

Register* REGISTER_SET (Value m_index, Value m_value);

Register* REGISTER_IND (Value m_index, Value m_value, Value m_value_index);

Register* REGISTER_EQ (Value m_value1, Value m_value2, Value not_position);

Register* REGISTER_NEQ (Value m_value1, Value m_value2, Value not_position);

Register* REGISTER_CMP (Value m_value1, Value m_value2, Value not_position);

Register* REGISTER_NCMP (Value m_value1, Value m_value2, Value not_position);

Register* REGISTER_SYS (Value cmd);

Register* REGISTER_BIG (Value m_value1, Value m_value2, Value m_index);

Register* REGISTER_SMA (Value m_value1, Value m_value2, Value m_index);

Register* REGISTER_BEQ (Value m_value1, Value m_value2, Value m_index);

Register* REGISTER_SEQ (Value m_value1, Value m_value2, Value m_index);

Register* REGISTER_JUMP (Value position);

Register* REGISTER_PRINT (Value m_index);

Register* REGISTER_FLUSH (Value m_index);

Register* REGISTER_NEG (Value m_index);

Register* REGISTER_PUSH (Value m_index);

Register* REGISTER_POP (Value m_index);

Register* REGISTER_CODE (Value code);

int fr_run (const Registry* register_list);

#endif // INTERPRETER_H
