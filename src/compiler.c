#include "compiler.h"
#include "multisearcher.h"
#include "gnumber.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Trims beginning and end of &string
void trim (char** text)
{
    if (**text == '\0')
        return;

    // Trim begin
    while (**text <= ' ')
        (*text) ++;

    // Trim end
    for (size_t len = strlen (*text) - 1; (*text)[len] <= ' '; -- len)
        (*text)[len] = '\0';
}

void ctrim (char** text)
{
    size_t i, j;
    bool in_string = 0;
    bool in_char   = 0;

    for (i = 0; (*text)[i] != '\0'; ++ i)
    {
        if ((*text)[i] == '\\' && i >= 1 && (*text)[i-1] == '\\' && ++ i)
            continue;

        if ((*text)[i] == '"' && (i == 0 || (*text)[i-1] != '\\') && !in_char)
            in_string = !in_string;

        if ((*text)[i] == '\'' && (i == 0 || (*text)[i-1] != '\\') && !in_string)
            in_char = !in_char;

        if ((*text)[i] > ' ' || in_string || in_char)
            continue;

        for (j = i + 1; (*text)[j] != '\0'; ++ j)
            (*text)[j - 1] = (*text)[j];
        (*text)[j - 1] = '\0';
    }
}

size_t contains (char* text, char c)
{
    bool in_string = 0;
    bool in_char   = 0;

    for (size_t i = 0; text[i] != '\0'; ++ i)
    {
        if (text[i] == '\\' && i >= 1 && text[i-1] == '\\' && ++ i)
            continue;

        if (text[i] == '"' && (i == 0 || text[i-1] != '\\') && !in_char)
            in_string = !in_string;

        if (text[i] == '\'' && (i == 0 || text[i-1] != '\\') && !in_string)
            in_char = !in_char;

        if (text[i] == c && !in_string && !in_char)
            return i + 1;
    }
    return 0;
}

bool is_str_concat (char* str)
{
    char* text = malloc (strlen (str) + 1);
    strcpy (text, str);

    ctrim (&text);

    bool is_string = 0;

    for (size_t i = 0; text[i] != '\0'; ++ i)
    {
        // continue if double '\'
        if (text[i] == '\\' && i >= 1 && text[i-1] == '\\' && ++ i)
            continue;

        if (text[i] == '"' && (i == 0 || text[i-1] != '\\'))
            is_string = !is_string;
        else if (!is_string)
        {
            free (text);
            return 1;
        }
    }

    free (text);

    return 0;
}

// Splits a string with a given delim saved in a *string passed as reference
// TODO: double '\\' does not work!
int split (char* buffer, char delim, char*** output)
{
    int  partCount = 0; // Count of splited elements used as index for ´output´
    int  inBracket = 0; // If ´inBracket´ bigger than 0 then ignore delim
    bool inString  = 0; // If ´inString´ eq 1 then ignore delim & brackets
    bool inChar    = 0; // If ´inChar´ eq 1 then ignore delim & brackets

    char* ptr;
    char* lastPos;

    buffer[strlen(buffer)] = '\0'; // Set buffers string end to end of string
    (*output) = malloc (1); // Allocate size

    // Loop until end of buffer is reached
    for (ptr = buffer, lastPos = buffer; *ptr != '\0'; ++ ptr)
    {
        // continue if double '\'
        if (*ptr == '\\' && *(ptr - 1) == '\\' && ++ ptr)
            continue;

        // Check if char is string
        if (*ptr == '\"' && *(ptr - 1) != '\\' && !inChar)
            inString = !inString;

        // Check if char is character
        if (*ptr == '\'' && *(ptr - 1) != '\\' && !inString)
            inChar = !inChar;

        // Check if it is the start of a bracket
        if (*ptr == '(' && !inString && !inChar)
            inBracket ++;

        // Check if it is the end of a bracket
        if (*ptr == ')' && !inString && !inChar)
            inBracket --;

        // Ignore delim if it is in a String/Char/Bracket
        if (*ptr != delim || inString || inChar || inBracket)
            continue;

        // Set splited substring in output and trim it
        (*output)[partCount] = lastPos;
        (*output)[partCount][ptr-lastPos] = '\0';
        trim (&(*output)[partCount]);        

        // Count ´partCount´ up
        partCount ++;
        
        // Set ´lastPos´ which is used to generate substr
        lastPos = ptr + 1;

        // Realloc memory to string -> +1 char
        (*output) = realloc (*output, sizeof (char*) * (partCount + 1));
    }

    // Set splited substring in output and trim it
    (*output)[partCount] = lastPos;
    trim (&(*output)[partCount]);

    // Return ´partCount´ used to iterate through substr list
    return partCount + 1;
}

char* str_replace (char* orig, char* rep, char* with) 
{
    char* result;  // the return string
    char* ins;     // the next insert point
    char* tmp;     // varies
    int len_rep;   // length of rep (the string to remove)
    int len_with;  // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;     // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;

    len_rep = strlen(rep);

    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count

    if (!with)
        with = "";

    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count)
        ins = tmp + len_rep;

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) 
    {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

Registry* register_list;

// Create ´register_list´ which is used to save the compiled commands
void fr_compiler_init ()
{
    fr_register_create (&register_list);
}

// Run the saved commands in ´register_list´
void fr_compiler_run ()
{
    fr_run (register_list);
    free (register_list);
}

// if not 0 then fr_compile is in a function
size_t in_function = 0;

// Here the code will be compiled into a list of registers -> ´register_list´
int fr_compile (const char* code, Variable** variables, const size_t pre_variable_count)
{
    size_t variable_count = pre_variable_count;

    CmsTemplate* cms_template;

    // Returns the index of variable by given name
    int var_get_pos_by_name (const char* name)
    {
        for (size_t i = 0; i < variable_count; ++ i)
           if ((*variables)[i].name[0] != '\0' && !strcmp ((*variables)[i].name, name))
                return (*variables)[i].position;
        return -1;
    }

    // Add new variable to list ´variables´
    size_t var_add (char* name, size_t m_index)
    {
        (*variables) = realloc (*variables, sizeof (Variable) * (variable_count + 1));

        if (!(*variables)) 
        {
            fprintf (stderr, "Failed to add new variable ´%s´ to variables, ´realloc´ failed!", name);
            exit (EXIT_FAILURE);
        }

//        printf ("Alloc %s at %d!\n", name, m_index);

        (*variables)[variable_count].position = m_index; 
        (*variables)[variable_count].name = name; 

        variable_count ++;

        return m_index;
    }

    // Doesn't work with ´\´ and ´\\´
    Value create_filled_in_str (char* text)
    {
        text = str_replace (str_replace (str_replace (str_replace (str_replace (str_replace (text, "\\\\", "$/638$"), "\\\"", "\""), "\\r", "\r"), "\\t", "\t"), "\\n", "\n"), "$/638$", "\\");

        bool   has_variables = 0;
        size_t m_index, i, j;

        char* varname;
        char *cvar, *tvar;

        for (i = 0; text[i] != '\0'; ++ i)
        {
            // continue if double '\'
            if (text[i] == '\\' && i >= 1 && text[i-1] == '\\' && ++ i)
                continue;

            if (text[i] != '$' || !(i == 0 || text[i-1] != '\\'))
                continue;

            varname = "";

            for (j = 0; j < variable_count; ++ j)
            {
                cvar = (*variables)[j].name;

                if (cvar[0] == '\0' || strlen (cvar) > strlen (text + i + 1))
                    continue;
                tvar = _substr (text, i + 1, i + 1 + strlen (cvar));
                if (!strcmp (cvar, tvar) && strlen (varname) < strlen (cvar))
                    varname = cvar;
                free (tvar);
            }

            if (varname[0] == '\0')
                continue;

            if (!has_variables)
            {
                m_index = fr_register_add (&register_list, REGISTER_ALLOC (VALUE_STR ("")));
                has_variables = 1;
            }

            text[i] = '\0';

            fr_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), VALUE_STR (text)));
            fr_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), POINTER (var_get_pos_by_name (varname))));

            text += i + 1 + strlen (varname);
            i = 0;
        }

        if (has_variables)
        {
            if (text[0] != '\0')
                fr_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), VALUE_STR (text)));
            return POINTER (m_index);
        }
        return VALUE_STR (text);
    }


    Value create_call_function (char* func_name, char* func_args, Value (fr_convert_to_value) (char* text))
    {
        // printf (">>> %s ( %s ) ;\n", func_name, func_args);

        char** args;
        size_t length = 0; 

        if (func_args != NULL && func_args[0] != '\0')
            length = split (func_args, ',', &args);

        Value position = fr_convert_to_value (func_name); 

        size_t m_tmp_var;

        // Arguments passed over call
        for (size_t i = 0; i < length; ++ i) 
        {
            m_tmp_var = fr_register_add (&register_list, REGISTER_ALLOC (position));
            fr_register_add (&register_list, REGISTER_SUB (VALUE_INT (m_tmp_var), VALUE_INT (i + 4)));
            fr_register_add (&register_list, REGISTER_PUSH (POINTER_POINTER (m_tmp_var))); // pushes variable to stack, used later to reset variable
            fr_register_add (&register_list, REGISTER_SET (POINTER (m_tmp_var), fr_convert_to_value (args[i])));
//            puts ("push-set");
        }

  //          puts ("push-fun");
        // Argument for location of call ´__origin__´
        size_t m_tmp = fr_register_add (&register_list, REGISTER_ALLOC (position));
        fr_register_add (&register_list, REGISTER_SUB (VALUE_INT (m_tmp), VALUE_INT (3)));
        fr_register_add (&register_list, REGISTER_PUSH (POINTER_POINTER (m_tmp))); // pushes variable to stack, used later to reset variable
        fr_register_add (&register_list, REGISTER_SET (POINTER (m_tmp), VALUE_INT (fr_get_current_register_position (&register_list) + 2)));

        // Calling jump
        fr_register_add (&register_list, REGISTER_JMP (position));

        // pops old variable __origin__ from stack 
        fr_register_add (&register_list, REGISTER_POP (POINTER (m_tmp)));
    //        puts ("pop-fun");

        // pops variable to stack, used later to reset variable
        for (int i = 0; i < length; ++ i) 
        {
            m_tmp_var = fr_register_add (&register_list, REGISTER_ALLOC (position));
            fr_register_add (&register_list, REGISTER_SUB (VALUE_INT (m_tmp_var), VALUE_INT (i + 4)));
            fr_register_add (&register_list, REGISTER_POP (POINTER (m_tmp_var)));
      //      puts ("pop");
        }

        // Return value
        m_tmp_var = fr_register_add (&register_list, REGISTER_ALLOC (position));
        m_tmp_var = fr_register_add (&register_list, REGISTER_ALLOC (POINTER_POINTER (m_tmp_var)));

        return POINTER (m_tmp_var);
    }

    // Converts a string to ´Value´ supports also different types of Values
    // TODO: Improve system
    Value fr_convert_to_value (char* text)
    {
        // Trim text to remove spaces
        trim (&text);

        if (!strcmp (text, "true"))
            return VALUE_INT (1);
        else if (!strcmp (text, "false"))
            return VALUE_INT (0);

        // ´text´ is a ´string´
        if (!is_str_concat (text))
        {
            text ++; // Remove first ´"´
            text[strlen (text) - 1] = '\0'; // Remove last ´"´
            return create_filled_in_str(text);
        }

        // ´text´ is a ´char´
        if ((text[0] == '\'' && text[strlen (text) - 1] == '\''))
        {
            text ++; // Remove first ´'´
            text[strlen (text) - 1] = '\0'; // Remove last ´'´
            return VALUE_CHAR (str_replace (str_replace (str_replace (str_replace (str_replace (str_replace (str_replace (text, "\\\\", "$/638$"), "\\'", "'"), "\\r", "\r"), "\\t", "\t"), "\\n", "\n"), "$/638$", "\\"), "\\'", "'")[0]);
        }

        // ´text´ is a variable and returns value -> ´pointer´
        int var_position;
        if ((var_position = var_get_pos_by_name (text)) != -1)
            return POINTER (var_position);
       
        // ´text´ is a variable and returns position ´int´ of variable
        if (text[0] == '&')
        {
            char* tmp_text = malloc (strlen (text));
            strcpy (tmp_text, text);
            tmp_text ++;
        
            trim (&tmp_text);

            if ((var_position = var_get_pos_by_name (tmp_text)) != -1)
                return VALUE_INT (var_position);
        }
        else if (text[0] == '~')
        {
            char* tmp_text = malloc (strlen (text));
            strcpy (tmp_text, text);
            tmp_text ++;
        
            trim (&tmp_text);

            if ((var_position = var_get_pos_by_name (tmp_text)) != -1)
                return POINTER_POINTER (var_position);
        }
        else if (text[0] == '!')
        {
            char* tmp_text = malloc (strlen (text));
            strcpy (tmp_text, text);
            tmp_text ++;
        
            trim (&tmp_text);

            if ((var_position = var_get_pos_by_name (tmp_text)) != -1)
            {
                size_t x = fr_register_add (&register_list, REGISTER_ALLOC (POINTER (var_position)));
                fr_register_add (&register_list, REGISTER_NEG (VALUE_INT (x)));
                return POINTER (x);
            }
        }

        // Check if text is a ´int´ or ´float´
        bool var_is_number = 1;
        bool var_is_float  = 1;

        for (size_t i = 0; text[i] != '\0'; ++ i)
        {
            if ((text[i] == '-' || text[i] == '+') && i == 0) // skip if first char is + or -
                continue;

            if (text[i] < '0' || text[i] > '9')
                var_is_number = 0;
            if ((text[i] < '0' || text[i] > '9') && text[i] != '.')
                var_is_float = 0;
        }

        // ´text´ is ´int´
        if (var_is_number)
            return VALUE_INT (atoi (text));

        // ´text´ is ´float´
        if (var_is_float)
            return VALUE_FLOAT (atof (text));


        // Function
        size_t func_end_name = 0;

        if ((func_end_name = contains (text, '(')) && strlen (text) == cms_find_next_bracket (func_end_name - 1, text) + 1)
        {
            char* func_name = malloc (func_end_name);
            strcpy (func_name, text);
            func_name[func_end_name - 1] = '\0';
            trim (&func_name);

            char* func_args = malloc (strlen (text) - func_end_name);
            strcpy (func_args, text + func_end_name);
            func_args[strlen (text) - func_end_name - 1] = '\0';
            trim (&func_args);

            // Check if function exist
            if (var_get_pos_by_name (func_name) != -1)
                return create_call_function (func_name, func_args, fr_convert_to_value);
        }

        // TODO: CALCULATING check if text contain +/-*
        return gna_registry_calculation_simple (&register_list, text, fr_convert_to_value);
    }



    void c_alloc (CmsData* data, int size)
    {
        Value  m_value = fr_convert_to_value (data[1]);
        var_add (data[0], fr_register_add (&register_list, REGISTER_ALLOC (m_value)));
    }

    void c_clabel (CmsData* data, int size)
    {
        var_add (data[0], fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (fr_get_current_register_position (&register_list)))));
    }

    void c_jlabel (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_JMP (fr_convert_to_value (data[0])));
    }



    void c_set (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_SET (VALUE_INT (var_get_pos_by_name (data[0])), fr_convert_to_value (data[1])));
    }

    void c_add (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_ADD (VALUE_INT (var_get_pos_by_name (data[0])), fr_convert_to_value (data[1])));
    }

    void c_sub (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_SUB (VALUE_INT (var_get_pos_by_name (data[0])), fr_convert_to_value (data[1])));
    }

    void c_mul (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_MUL (VALUE_INT (var_get_pos_by_name (data[0])), fr_convert_to_value (data[1])));
    }

    void c_div (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_DIV (VALUE_INT (var_get_pos_by_name (data[0])), fr_convert_to_value (data[1])));
    }



    void c_set_pointer (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_SET (fr_convert_to_value (data[0]), fr_convert_to_value (data[1])));
    }

    void c_add_pointer (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_ADD (fr_convert_to_value (data[0]), fr_convert_to_value (data[1])));
    }

    void c_sub_pointer (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_SUB (fr_convert_to_value (data[0]), fr_convert_to_value (data[1])));
    }

    void c_mul_pointer (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_MUL (fr_convert_to_value (data[0]), fr_convert_to_value (data[1])));
    }

    void c_div_pointer (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_DIV (fr_convert_to_value (data[0]), fr_convert_to_value (data[1])));
    }



    void c_print (CmsData* data, int size)
    {
        char** args;

        size_t length = split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_OUT (fr_convert_to_value (args[i])));

        free (args);
    }

    void c_input (CmsData* data, int size)
    {
        char** args;

        size_t length = split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_CIN (fr_convert_to_value (args[i])));

        free (args);
    }

    void c_system (CmsData* data, int size)
    {
        char** args;

        size_t length = split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_SYS (fr_convert_to_value (args[i])));

        free (args);
    }

    void c_push (CmsData* data, int size)
    {
        char** args;

        size_t length = split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_PUSH (fr_convert_to_value (args[i])));

        free (args);
    }

    void c_pop (CmsData* data, int size)
    {
        char** args;

        size_t length = split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_POP (fr_convert_to_value (args[i])));

        free (args);
    }

    void c_check (CmsData* data, int size)
    {
        Value  m_value = fr_convert_to_value (data[0]);
        size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = fr_register_add (&register_list, REGISTER_NEQ (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        fr_compile (data[1], variables, variable_count);
        register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list));
    }

    void c_ncheck (CmsData* data, int size)
    {
        Value  m_value = fr_convert_to_value (data[0]);
        size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = fr_register_add (&register_list, REGISTER_EQ (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        fr_compile (data[1], variables, variable_count);
        register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list));
    }

    void c_loop (CmsData* data, int size)
    {
        size_t loop_index = fr_get_current_register_position (&register_list);

        Value  m_value = fr_convert_to_value (data[0]);
        size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = fr_register_add (&register_list, REGISTER_NEQ (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        fr_compile (data[1], variables, variable_count);
        fr_register_add (&register_list, REGISTER_JMP (VALUE_INT (loop_index)));
        register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list));
    }

    void c_check_else (CmsData* data, int size)
    {
        Value  m_value = fr_convert_to_value (data[0]);
        size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = fr_register_add (&register_list, REGISTER_NEQ (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        {
            fr_compile (data[1], variables, variable_count);
        }
        size_t y = fr_register_add (&register_list, REGISTER_JMP (VALUE_INT (0)));
        register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list));
        {
            fr_compile (data[2], variables, variable_count);
        }
        register_list[y]->reg_values[0] = VALUE_INT (fr_get_current_register_position (&register_list));
    }

    void c_ncheck_else (CmsData* data, int size)
    {
        Value  m_value = fr_convert_to_value (data[0]);
        size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = fr_register_add (&register_list, REGISTER_EQ (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        {
            fr_compile (data[1], variables, variable_count);
        }
        size_t y = fr_register_add (&register_list, REGISTER_JMP (VALUE_INT (0)));
        register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list));
        {
            fr_compile (data[2], variables, variable_count);
        }
        register_list[y]->reg_values[0] = VALUE_INT (fr_get_current_register_position (&register_list));
    }

    void c_function (CmsData* data, int size)
    {
        char** args;
        char*  var_name;

        // argument length
        size_t length = 0;

        // split arguments of function
        if (((char*)data[1])[0] != '\0')
            length = split (data[1], ',', &args);

        Value  m_value;

        size_t equal_pos;
        size_t func_pos;

        // recieved parameters
        for (int i = length - 1; i >= 0; -- i) 
        {
            // check if contains an ´=´
            if (equal_pos = contains (args[i], '='))
            {
                args[i][equal_pos - 1] = '\0';
                sprintf (var_name = malloc (strlen (data[0]) + equal_pos + 1), "%s.%s", data[0], args[i]);
                m_value = fr_convert_to_value (args[i] + equal_pos); // - 1
                ctrim (&var_name); // trim spaces
                var_add (var_name, fr_register_add (&register_list, REGISTER_ALLOC (m_value)));
            }
            else
            {
                sprintf (var_name = malloc (strlen (data[0]) + strlen (args[i]) + 2), "%s.%s", data[0], args[i]);
                var_add (var_name, fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0/*-2*/))));
            }
        }

        // first default argument, ´__origin__´ used to determine where to go back
        sprintf (var_name = malloc (strlen (data[0]) + 11 + 1), "%s.__origin__", data[0]);
        var_add (var_name, fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (-1))));


        // Allocate memory for function position
        func_pos = fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (fr_get_current_register_position (&register_list) + 2)));
        var_add (data[0], func_pos);

        // Jump to end of functions body, will not happen if function is called
        size_t x = fr_register_add (&register_list, REGISTER_JMP (VALUE_INT (-1)));

        // Allocate memory for function return
        fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));

        // Compile function body
        size_t old_in_function = in_function;
        in_function = func_pos;
puts (data[0]);
        fr_compile (data[2], variables, variable_count);
puts (data[0]);
        in_function = old_in_function;
        

        // Jump to call position (location of call saved in ´__origin__´)
        fr_register_add (&register_list, REGISTER_JMP (fr_convert_to_value (var_name)));

        // Set skip jump to current location of register
        register_list[x]->reg_values[0] = VALUE_INT (fr_get_current_register_position(&register_list));
    }

    void c_function_short (CmsData* data, int size)
    {
        data[2] = realloc (data[2], strlen (data[2]) + 2);
        ((char*)data[2])[strlen(data[2])] = ';';
        ((char*)data[2])[strlen(data[2]) + 1] = '\0';
        c_function (data, size);
    }

    void c_call (CmsData* data, int size)
    {
        create_call_function (data[0], data[1], fr_convert_to_value);
    }

    void c_return (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_SET (VALUE_INT (in_function + 2), fr_convert_to_value (data[0])));
        fr_register_add (&register_list, REGISTER_JMP (POINTER (in_function - 1)));
    }

    // Check if c_check_else & c_check can be replaced with c_bracket...
    cms_create ( &cms_template, CMS_LIST ( {
        cms_add ("# ( % ) { % }",          c_function,          CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# ( % ) > % ;",          c_function_short,    CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# ( % ) ;",   c_call,    CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("var # = % ;", c_alloc,   CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # += % ;",  c_add_pointer,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # -= % ;",  c_sub_pointer,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # *= % ;",  c_mul_pointer,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # /= % ;",  c_div_pointer,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # = % ;",   c_set_pointer,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# += % ;",    c_add,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# -= % ;",    c_sub,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# *= % ;",    c_mul,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# /= % ;",    c_div,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# = % ;",     c_set,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("O: % ;",      c_print,   CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("I: % ;",      c_input,   CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("S: % ;",      c_system,  CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("PUSH: % ;",   c_push,    CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("POP: % ;",    c_pop,     CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("jump % ;",    c_jlabel,  CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("#:",          c_clabel,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! ( % ) { % } { % }", c_ncheck_else,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! ( % ) { % }",       c_ncheck,       CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! # { % } { % }",     c_ncheck_else,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! # { % }",           c_ncheck,       CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) { % } { % }",   c_check_else,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) { % }",         c_check,        CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# { % } { % }",       c_check_else,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# { % }",             c_check,        CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) -> { % }",      c_loop,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# -> { % }",          c_loop,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("ret % ;",             c_return,       CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
    } ));

    // Search syntax using ´cms_template´ in ´example_text´
    cms_find (code, cms_template);

    (*variables) = realloc (*variables, sizeof (Variable) * (pre_variable_count));

    return EXIT_SUCCESS;
}
