#include "parser.h"
#include "multisearcher.h"
#include "gnumber.h"
#include "library.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Registry* register_list = NULL;

// Create ´register_list´ which is used to save the compiled commands
void kl_parse_compiler_init ()
{
    if (register_list != NULL)
        free (register_list);
    register_list = NULL;
    kl_intp_register_create (&register_list);
}

// Run the saved commands in ´register_list´
void kl_parse_compiler_run ()
{
    kl_intp_run (register_list);
    free (register_list);
    register_list = NULL;
}

// in_function > 0 then kl_parse_compile is in a ´function´
// in_function = 0 then kl_parse_compile is in ´main´
// in_function < 0 then kl_parse_compile is in ´scope´
int in_function = 0;

// Used to set variable name;
char* function_path = NULL; // local.

// scope, ret back jump position stored in array to set later -> TODO: fix bug if multiple ´scope´ ret
size_t scope_jump_back[100];
size_t scope_jump_back_size = 0;

void kl_parse_add_variable (Variable** variables, size_t* variable_count, const char* path, const char* name, const bool constant, Value value)
{
    if (name[0] == '\0')
        error ("Variable Name has to be at least 1 char long!", NULL);

    if (name[0] >= '0' && name[0] <= '9')
        error ("Variable ´%s´ cannot start with a number!", name);

    for (int i = *variable_count - 1; i >= 0; -- i)
        if (!strcmp ((*variables)[i].name, name) && !strcmp ((*variables)[i].function_path, path) && strcmp ((*variables)[i].name, "__origin__"))
            error ("Variable ´%s´ already exists in this scope!", name);

    (*variables) = realloc (*variables, sizeof (Variable) * (*variable_count + 1));

    if (!(*variables))
        error ("Failed to add new variable ´%s´ to variables, ´realloc´ failed!", name);

    (*variables)[*variable_count].position = kl_intp_register_add (&register_list, REGISTER_ALLOC (value));
    (*variables)[*variable_count].constant = constant;
    strcpy ((*variables)[*variable_count].name = malloc (strlen (name) + 1), name);
    strcpy ((*variables)[*variable_count].function_path = malloc (strlen (path) + 1), path); 

    (*variable_count) ++;
}

// Here the code will be compiled into a list of registers -> ´register_list´
int kl_parse_compile (char* code, Variable** variables, size_t* pre_variable_count, const bool reset_variables)
{
    if (!code || !variables)
        return EXIT_SUCCESS;

    // Filter comments out of code -> TODO: Move comment out of ´kl_parse_compile´ -> performance
    kl_util_filter_comment (&code);

    // Add ´local.´ to function_path if ´kl_parse_compile´ is main
    if (!function_path)
        strcpy (function_path = malloc (strlen ("local") + 1), "local");

    // Used to reset ´variables´ back to normal size at the end of ´kl_parse_compile´
    size_t variable_count = *pre_variable_count;

    // Template storing syntax, used for the CMultiSearcher
    CmsTemplate* cms_template;

    // Returns the index of variable by given name
    int var_get_pos_by_name (char* name, bool wants_to_modify)
    {
        kl_util_ctrim (&name);

        Variable variable;
        char* full_var_name;
        int i;

        // Variable has full varname path beginning from ´local.´
        for (i = variable_count - 1; i >= 0; -- i)
        {
            full_var_name = malloc (strlen ((*variables)[i].function_path) + strlen ((*variables)[i].name) + 2);
            sprintf (full_var_name, "%s.%s", (*variables)[i].function_path, (*variables)[i].name);

            bool exist = !strcmp (full_var_name + strlen ("local") + 1, name);

            free (full_var_name);
            full_var_name = NULL;

            if (!exist)
                continue;

            variable = (*variables)[i];
            goto var_get_pos_by_name_return;
        }

        // Variable has full varname path
        for (i = variable_count - 1; i >= 0; -- i)
        {
            full_var_name = malloc (strlen ((*variables)[i].function_path) + strlen ((*variables)[i].name) + 2);
            sprintf (full_var_name, "%s.%s", (*variables)[i].function_path, (*variables)[i].name);

            bool exist = !strcmp (full_var_name, name);

            free (full_var_name);
            full_var_name = NULL;

            if (!exist)
                continue;

            variable = (*variables)[i];
            goto var_get_pos_by_name_return;
        }

        // Variable in same scope
        for (i = variable_count - 1; i >= 0; -- i)
        {
            if (strcmp ((*variables)[i].name, name) || strcmp ((*variables)[i].function_path, function_path))
                continue;

            variable = (*variables)[i];
            goto var_get_pos_by_name_return;
        }

        // Variable not in same scope
        for (i = variable_count - 1; i >= 0; -- i)
        {
            if (strcmp ((*variables)[i].name, name))
                continue;

            variable = (*variables)[i];
            goto var_get_pos_by_name_return;
        }

        return UNKNOWN;

        var_get_pos_by_name_return: 

        if (variable.constant && wants_to_modify)
            error ("Variable ´%s´ is constant!", variable.name);
            
        return variable.position;
    }

    // Add new variable to list ´variables´
    size_t var_add_function_path (const char* path, const char* name, const size_t m_index, const bool constant)
    {
        if (name[0] == '\0')
            error ("Variable Name has to be at least 1 char long!", NULL);

        if (name[0] >= '0' && name[0] <= '9')
            error ("Variable ´%s´ cannot start with a number!", name);

        for (int i = variable_count - 1; i >= 0; -- i)
            if (!strcmp ((*variables)[i].name, name) && !strcmp ((*variables)[i].function_path, path) && strcmp ((*variables)[i].name, "__origin__"))
                error ("Variable ´%s´ already exists in this scope!", name);

        if (kl_lib_get_function_by_name (name) != UNKNOWN)
            error ("Function ´%s´ already exists in library!", name);
            

        (*variables) = realloc (*variables, sizeof (Variable) * (variable_count + 1));

        if (!(*variables))
            error ("Failed to add new variable ´%s´ to variables, ´realloc´ failed!", name);

        (*variables)[variable_count].position = m_index; 
        (*variables)[variable_count].constant = constant;
        strcpy ((*variables)[variable_count].name = malloc (strlen (name) + 1), name);
        strcpy ((*variables)[variable_count].function_path = malloc (strlen (path) + 1), path); 

        variable_count ++;

        return m_index;
    }

    // Remove new variable to list ´variables´
    void var_rem () { (*variables) = realloc (*variables, sizeof (Variable) * (variable_count -= 1)); }

    void var_add (const char* name, const size_t m_index, const bool constant) { var_add_function_path (function_path, name, m_index, constant); }

    // Doesn't work with ´\´ and ´\\´
    // Used for special string inserts example: "${}" or "$VARIABLE"
    Value create_filled_in_str (char* text, Value (kl_parse_convert_to_value) (char* text))
    {
        text = kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (text, "\\\\", "$/638$"), "\\\"", "\""), "\\r", "\r"), "\\t", "\t"), "\\x1b", "\x1b"), "\\n", "\n"), "$/638$", "\\");

        bool   has_variables = 0;
        size_t m_index, i, j;

        char *varname;
        char *cvar, *tvar;

        for (i = 0; text[i] != '\0'; ++ i)
        {
            // continue if double '\'
            if (text[i] == '\\' && i >= 1 && text[i-1] == '\\' && ++ i)
                continue;

            if (text[i] != '$' || !(i == 0 || text[i-1] != '\\'))
                continue;

            if (text[i + 1] == '{')
            {
                int last_bracket = kl_util_find_next_bracket (++ i, text);

                if (!has_variables)
                {
                    m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_STR ("")));
                    has_variables = 1;
                }

                text[i - 1] = '\0';
                text[i] = '\0';

                kl_intp_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), VALUE_STR (text)));
                kl_intp_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), kl_parse_convert_to_value (kl_util_substr (text+i+1, 0, last_bracket - i - 1))));

                text += last_bracket + 1;
                i = -1;
            }
            else
            {
                varname = "";

                for (j = 0; j < variable_count; ++ j)
                {
                    cvar = (*variables)[j].name;

                    if (cvar[0] == '\0' || strlen (cvar) > strlen (text + i + 1))
                        continue;
                    tvar = kl_util_substr (text, i + 1, i + 1 + strlen (cvar));
                    if (!strcmp (cvar, tvar) && strlen (varname) < strlen (cvar))
                        varname = cvar;
                    free (tvar);
                    tvar = NULL;
                }

                if (varname[0] == '\0')
                    continue;

                if (!has_variables)
                {
                    m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_STR ("")));
                    has_variables = 1;
                }

                text[i] = '\0';

                kl_intp_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), VALUE_STR (text)));
                kl_intp_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), POINTER (var_get_pos_by_name (varname, false))));

                text += i + 1 + strlen (varname);
                i = -1;
            }
        }

        if (has_variables)
        {
            if (text[0] != '\0')
                kl_intp_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), VALUE_STR (text)));
            return POINTER (m_index);
        }
        return VALUE_STR (text);
    }

    Value call_lib_function (int id, char* func_args, Value (kl_parse_convert_to_value) (char* text))
    {
        if (id == UNKNOWN)
            return VALUE_INT (0);

        size_t m_index_ret = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));

        char** args = NULL;
        size_t args_length = kl_util_split (func_args, ',', &args);

        Value* args_values = malloc (sizeof (Value) * args_length);

        for (size_t i = 0; i < args_length; ++  i)
            args_values[i] = kl_parse_convert_to_value (args[i]);

        if (args != NULL)
            free (args);

        kl_intp_register_add (&register_list, REGISTER_LIB_FUNC (
            VALUE_INT (id),
            VALUE_INT (m_index_ret),
            VALUE_LIST (args_values, args_length)
        ));

        return POINTER (m_index_ret);
    }

    // TODO: WRONG POSITION FOR ARGUMENTS
    Value create_call_function_args (char* func_name, Value* args, size_t length, Value (kl_parse_convert_to_value) (char* text))
    {
        Value position = kl_parse_convert_to_value (func_name); 

        size_t m_tmp_var;

        int func_args_positions[length];
        int func_args_values[length];

        // Arguments passed over call
        for (size_t i = 0; i < length; ++ i) 
        {
            func_args_positions[i] = m_tmp_var = kl_intp_register_add (&register_list, REGISTER_ALLOC (position));
            kl_intp_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_tmp_var), VALUE_INT (i + 2)));
            kl_intp_register_add (&register_list, REGISTER_PUSH (POINTER_POINTER (m_tmp_var))); // pushes variable to stack, used later to reset variable
            func_args_values[i] = kl_intp_register_add (&register_list, REGISTER_ALLOC (args[i]));
        }

        for (size_t i = 0; i < length; ++ i)
            kl_intp_register_add (&register_list, REGISTER_SSET (POINTER (func_args_positions[i]), POINTER (func_args_values[i])));

        // Argument for location of call ´__origin__´
        size_t m_tmp = kl_intp_register_add (&register_list, REGISTER_ALLOC (position));
        kl_intp_register_add (&register_list, REGISTER_PUSH (POINTER_POINTER (m_tmp))); // pushes variable to stack, used later to reset variable
        kl_intp_register_add (&register_list, REGISTER_SSET (POINTER (m_tmp), VALUE_INT (kl_intp_get_current_register_position (&register_list) + 2)));

        // Calling jump
        kl_intp_register_add (&register_list, REGISTER_JUMP (position));

        // pops old variable __origin__ from stack 
        kl_intp_register_add (&register_list, REGISTER_POP (POINTER (m_tmp)));

        // pops variable to stack, used later to reset variable
        for (int i = length - 1; i >= 0; -- i) 
            kl_intp_register_add (&register_list, REGISTER_POP (POINTER (func_args_positions[i])));

        // Return value
        m_tmp_var = kl_intp_register_add (&register_list, REGISTER_ALLOC (position));
        kl_intp_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_tmp_var), VALUE_INT (1)));
        m_tmp_var = kl_intp_register_add (&register_list, REGISTER_ALLOC (POINTER_POINTER (m_tmp_var)));

        free (args);

        return POINTER (m_tmp_var);
    }

    Value create_call_function (char* func_name, char* func_args, Value (kl_parse_convert_to_value) (char* text))
    {
        size_t fstream = kl_util_contains (func_name, ':');

        if (fstream && func_name[fstream] == ':')
        {
            char* data_name = kl_util_substr (func_name, 0, fstream - 1);
            char* data_args = malloc (strlen (data_name) + 1 + strlen (func_args) + 1);
            
            *data_args = '\0';
            strcat (data_args, data_name);

            if (func_args[0] != '\0')
            {
                strcat (data_args, ",");
                strcat (data_args, func_args);
            }

            // New function arguments
            strcpy (func_args = realloc (func_args, strlen (data_args) + 1), data_args);

            // New function name
            func_name += strlen (data_name);

            free (data_name);
            free (data_args);
        }

        int kl_lib_id = UNKNOWN;

        // Check if function exist
        if ((kl_lib_id = kl_lib_get_function_by_name (func_name)) != UNKNOWN)
            return call_lib_function (kl_lib_id, func_args, kl_parse_convert_to_value);

        char** args;
        size_t length = 0; 

        if (func_args != NULL && func_args[0] != '\0')
            length = kl_util_split (func_args, ',', &args);

        Value* values = malloc (sizeof (Value) * length);

        for (size_t i = 0; i < length; ++ i)
            values[i] = kl_parse_convert_to_value (args[i]);

        return create_call_function_args (func_name, values, length, kl_parse_convert_to_value);
    }

    Value create_call_function_value (char* func_name, Value arg_value, char* func_args, Value (kl_parse_convert_to_value) (char* text))
    {
        char** args = NULL;
        size_t length = 0; 

        if (func_args != NULL && func_args[0] != '\0')
            length = kl_util_split (func_args, ',', &args);     // +1 for ´arg_value´

        Value* values = malloc (sizeof (Value) * length);

        values[0] = arg_value;

        for (size_t i = 0; i < length; ++ i)
            values[i + 1] = kl_parse_convert_to_value (args[i]);
        return create_call_function_args (func_name, values, length + 1, kl_parse_convert_to_value);
    }

    bool is_illegal_name (const char* name)
    {
        return !strcmp (name, "var") 
            || !strcmp (name, "val") 
            || !strcmp (name, "ret") 
            || !strcmp (name, "push") 
            || !strcmp (name, "pop") 
            || !strcmp (name, "inc")
            || kl_util_has_illigal_ascii (name);
    }

    Value create_function (char* func_name, char* func_args, char* func_code, Value (kl_parse_convert_to_value) (char* text))
    {
        char** args;
        char*  var__origin__;

        // argument length
        size_t length = 0;

        char* old_function_path;
        strcpy (old_function_path = malloc (strlen (function_path) + 1), function_path);

        // Add to function_path
        if (func_name != NULL)
        {
            for (int i = variable_count - 1; i >= 0; -- i)
                if (!strcmp ((*variables)[i].name, func_name) && !strcmp ((*variables)[i].function_path, function_path))
                    error ("Function or Variable ´%s´ already exists in this scope!", func_name);
            if (is_illegal_name (func_name))
                error ("Function ´%s´ cannot be called like that!", func_name);

            strcat (function_path = realloc (function_path, strlen (function_path) + strlen (func_name) + 2), ".");
            strcat (function_path, func_name);
        }
        

        // Allocate memory for function position
        Value  value_func_position = VALUE_INT (kl_intp_get_current_register_position (&register_list) + 2);
        size_t func_pos = kl_intp_register_add (&register_list, REGISTER_ALLOC (value_func_position));

        if (func_name != NULL)
            var_add_function_path (old_function_path, func_name, func_pos, false);

        // Jump to end of functions body, will not happen if function is called
        size_t x = kl_intp_register_add (&register_list, REGISTER_JUMP (VALUE_INT (-1)));

        // first default argument, ´__origin__´ used to determine where to go back
        sprintf (var__origin__ = malloc (10 + 1), "__origin__");
        var_add (var__origin__, kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (-1))), false);

        // Allocate memory for function return
        kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));

        // kl_util_split arguments of function
        if (func_args[0] != '\0')
            length = kl_util_split (func_args, ',', &args);

        Value  m_value;
        size_t equal_pos;

        // recieved parameters
        for (size_t i = 0; i < length; ++ i) 
        {
            // check if kl_util_contains an ´=´
            if ((equal_pos = kl_util_contains (args[i], '=')))
            {
                args[i][equal_pos - 1] = '\0';
                m_value = kl_parse_convert_to_value (args[i] + equal_pos); // - 1
                kl_util_ctrim (&args[i]);
                var_add (args[i], kl_intp_register_add (&register_list, REGISTER_ALLOC (m_value)), false);
            }
            else
            {
                var_add (args[i], kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))), false);
            }
        }

        // Compile function body
        int old_in_function = in_function;
        in_function = func_pos;
        kl_parse_compile (func_code, variables, &variable_count, true);
        in_function = old_in_function;
         
        // Jump to call position (location of call saved in ´__origin__´)
        kl_intp_register_add (&register_list, REGISTER_JUMP (kl_parse_convert_to_value (var__origin__)));

        // Set skip jump to current location of register
        register_list[x]->reg_values[0] = VALUE_INT (kl_intp_get_current_register_position(&register_list));
 
        // Remove from function_path
        if (func_name != NULL)
        {
            size_t function_path_size = strlen (function_path) - strlen (func_name) - 1;
            function_path[function_path_size] = '\0';
            function_path = realloc (function_path, function_path_size + 1);
        }
    
        // Remove argument variables, outside not reachable!
        for (size_t i = 0; i < length; ++ i) 
            var_rem ();

        return value_func_position;
    }

    // Converts a code-string to ´Value´, it can recognise different datatypes of Values
    Value kl_parse_convert_to_value (char* text)
    {
        // Trim text begin & end
        kl_util_trim (&text);

        // Remove brackets if outside is bracket
        if (text[0] == '(' && (int) strlen (text) == kl_util_find_next_bracket (0, text) + 1)
        {
            kl_util_trim_front_end (&text); // remove start & end bracket
            kl_util_trim (&text);
        }

        // Create list
        if (text[0] == '[' && (int) strlen (text) == kl_util_find_next_bracket (0, text) + 1)
        {
            kl_util_trim_front_end (&text); // remove start & end bracket
            kl_util_trim (&text);

            char** args = NULL;
            size_t length = kl_util_split (text, ',', &args);

            Value* values = malloc (sizeof (Value) * length);

            for (size_t i = 0; i < length; ++  i)
                values[i] = kl_parse_convert_to_value (args[i]);

            if (args != NULL)
                free (args);

            int m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_LIST (values, length)));
            return POINTER (m_index);
        }

        // scope
        if (text[0] == '{' && (int) strlen (text) == kl_util_find_next_bracket (0, text) + 1) 
        {
            kl_util_trim_front_end (&text); // remove start & end bracket
            int m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))); // alloc memory for return
            int old_in_function = in_function;
            in_function = -m_index; // store old ´in_function´ and set new one (neg used for ´scope´ in ret)

            kl_parse_compile (text, variables, &variable_count, true); // compile code in bracket

            Value value_current_reg_pos = VALUE_INT (kl_intp_get_current_register_position (&register_list)); 
            for (byte i = 0; i < scope_jump_back_size; ++ i)
                register_list[scope_jump_back[i]]->reg_values[0] = value_current_reg_pos;
            scope_jump_back_size = 0;

            in_function = old_in_function; // restore old ´in_function´
            return POINTER (m_index); // return pointer to stored memory
        }

        // Boolean constant
        if (!strcmp (text, "true"))
            return VALUE_INT (true);
        else if (!strcmp (text, "false"))
            return VALUE_INT (false);

        // short if 
        bool is_if   = kl_util_contains (text, '?');
        bool is_else = kl_util_contains (text, ':');

        if (is_if && is_else)
        {
            text[is_if - 1] = '\0';    // set '\0' at '?'
            text[is_else  - 1] = '\0'; // set '\0' at ':'
            Value val_condition = kl_parse_convert_to_value (text); // create value for condition

            size_t m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (val_condition)); // alloc memory for return

            size_t x = kl_intp_register_add (&register_list, REGISTER_NCMP (POINTER (m_index), VALUE_INT (0), VALUE_INT (0))); // condition if
            {
                kl_intp_register_add (&register_list, REGISTER_SET (VALUE_INT (m_index), kl_parse_convert_to_value (text += is_if))); // set memory if true
            }
            size_t y = kl_intp_register_add (&register_list, REGISTER_JUMP (VALUE_INT (0))); // condition else
            register_list[x]->reg_values[2] = VALUE_INT (kl_intp_get_current_register_position (&register_list)); // set ´condition if´ jump point
            {
                kl_intp_register_add (&register_list, REGISTER_SET (VALUE_INT (m_index), kl_parse_convert_to_value (text += is_else - is_if))); // set memory value if false
            }
            register_list[y]->reg_values[0] = VALUE_INT (kl_intp_get_current_register_position (&register_list)); // set ´condition else´ jump point

            return POINTER (m_index); // return pointer of memory
        }

        // boolean algebra
        if (kl_util_contains (text, '&') || kl_util_contains (text, '|'))
            return gna_registry_boolean_algebra (&register_list, text, kl_parse_convert_to_value);

        size_t is_point = kl_util_contains (text, '.');

        // TODO: Step!
        if (is_point && text[is_point] == '.' && text[is_point + 1] == '.')
        {
            text[is_point - 1] = ',';
            text[is_point]     = ' ';
            text[is_point + 1] = ' ';
            return call_lib_function (kl_lib_get_function_by_name ("range"), text, kl_parse_convert_to_value);
        }
        else if (is_point && text[is_point] == '.')
        {
            text[is_point - 1] = ',';
            text[is_point]     = ' ';
            return call_lib_function (kl_lib_get_function_by_name ("until"), text, kl_parse_convert_to_value);
        }

        size_t is_bigger  = kl_util_contains (text, '>'); // >,  >=
        size_t is_smaller = kl_util_contains (text, '<'); // <,  <=
        size_t is_equal   = kl_util_contains (text, '='); // ==, !=
        size_t is_pow     = kl_util_contains (text, '^'); // ==, !=

        char* v1_text = NULL; // condition before compute
        char* v2_text = NULL; // condition after  compute

        if (is_pow)
        {
            v1_text = kl_util_substr (text, 0, is_pow - 1);
            v2_text = kl_util_substr (text, is_pow, strlen (text));

            int m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_FLOAT (0)));
            kl_intp_register_add (&register_list, REGISTER_POW (kl_parse_convert_to_value (v1_text), kl_parse_convert_to_value (v2_text), VALUE_INT (m_index)));

            free (v1_text);
            free (v2_text);

            return POINTER (m_index);
        }
        else if (is_bigger)
        {
            v1_text = kl_util_substr (text, 0, is_bigger - 1);
            v2_text = kl_util_substr (text, is_bigger + (text[is_bigger] == '=' ? 1 : 0), strlen (text));

            int m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
            kl_intp_register_add (&register_list, (text[is_bigger] == '=' ? REGISTER_BEQ : REGISTER_BIG) (kl_parse_convert_to_value (v1_text), kl_parse_convert_to_value (v2_text), VALUE_INT (m_index)));

            free (v1_text);
            free (v2_text);

            return POINTER (m_index);
        }
        else if (is_smaller)
        {
            v1_text = kl_util_substr (text, 0, is_smaller - 1);
            v2_text = kl_util_substr (text, is_smaller + (text[is_smaller] == '=' ? 1 : 0), strlen (text));

            int m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
            kl_intp_register_add (&register_list, (text[is_smaller] == '=' ? REGISTER_SEQ : REGISTER_SMA) (kl_parse_convert_to_value (v1_text), kl_parse_convert_to_value (v2_text), VALUE_INT (m_index)));

            free (v1_text);
            free (v2_text);

            return POINTER (m_index);
        }
        else if (is_equal && (text[is_equal - 2] == '+' || text[is_equal - 2] == '-' || text[is_equal - 2] == '*' || text[is_equal - 2] == '/'))
        {
            v1_text = kl_util_substr (text, 0, is_equal - 2);
            v2_text = kl_util_substr (text, is_equal, strlen (text));

            size_t m_index = var_get_pos_by_name (v1_text, true); 

            if (text[is_equal - 2] == '+')
                kl_intp_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), kl_parse_convert_to_value (v2_text)));
            else if (text[is_equal - 2] == '-')
                kl_intp_register_add (&register_list, REGISTER_SUB (VALUE_INT (m_index), kl_parse_convert_to_value (v2_text)));
            else if (text[is_equal - 2] == '*')
                kl_intp_register_add (&register_list, REGISTER_MUL (VALUE_INT (m_index), kl_parse_convert_to_value (v2_text)));
            else if (text[is_equal - 2] == '/')
                kl_intp_register_add (&register_list, REGISTER_DIV (VALUE_INT (m_index), kl_parse_convert_to_value (v2_text)));

            free (v1_text);
            free (v2_text);

            return POINTER (m_index);
        }
        else if (is_equal && (text[is_equal] == '=' || text[(is_equal -= 1) - 1] == '!'))
        {
            v1_text = kl_util_substr (text, 0, is_equal - 1);
            v2_text = kl_util_substr (text, is_equal + 1, strlen (text));

            int m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
            kl_intp_register_add (&register_list, (text[is_equal-1] == '=' ? REGISTER_EQ : REGISTER_NEQ) (kl_parse_convert_to_value (v1_text), kl_parse_convert_to_value (v2_text), VALUE_INT (m_index)));

            free (v1_text);
            free (v2_text);

            return POINTER (m_index);
        }

        if (!kl_util_is_str_concat (text)) // ´text´ is a ´string´
        {
            kl_util_trim_front_end (&text); // Remove last ´"´
            return create_filled_in_str(text, kl_parse_convert_to_value);
        }
        else if ((text[0] == '\'' && text[strlen (text) - 1] == '\'') && strlen (text) <= 6) // ´text´ is a ´char´
        {
            kl_util_trim_front_end (&text); // Remove last ´'´
            return VALUE_CHAR (kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (kl_util_str_replace (text, "\\\\", "$/638$"), "\\'", "'"), "\\r", "\r"), "\\t", "\t"), "\\x1b", "\x1b"), "\\n", "\n"), "$/638$", "\\"), "\\'", "'")[0]);
        }

        // Check if text is a ´int´ or ´float´
        bool var_is_number = true;
        bool var_is_float  = true;

        for (size_t i = 0; text[i] != '\0'; ++ i)
        {
            if ((text[i] == '-' || text[i] == '+') && i == 0) // skip if first char is + or -
                continue;

            if (text[i] == 'i' && text[i + 1] == '\0' && i != 0)
                var_is_float = false;
            else if (text[i] == 'l' && text[i + 1] == '\0' && i != 0)
                var_is_number = false;
            else 
            {
                if ((text[i] < '0' || text[i] > '9') && text[i] != '.')
                    var_is_float = false;
                if (text[i] < '0' || text[i] > '9')
                    var_is_number = false;
            }
        }

        // ´text´ is ´int´
        if (var_is_number)
            return VALUE_INT (atoi (text));
        else if (text[0] == '0' && text[1] == 'x')
            return VALUE_INT (strtol (text, NULL, 16));
        else if (text[0] == '#')
            return VALUE_INT (strtol (text + 1, NULL, 16));

        // ´text´ is ´float´
        if (var_is_float)
            return VALUE_FLOAT (atof (text));

        // calculation with ´+/-*
        if (kl_util_contains (text, '+') || 
            kl_util_contains (text, '-') || 
            kl_util_contains (text, '*') || 
            kl_util_contains (text, '/') || 
            kl_util_contains (text, '%'))
                return gna_registry_calculation_simple (&register_list, text, kl_parse_convert_to_value);

        // Function call
        size_t func_end_name = 0;

        if ((func_end_name = kl_util_contains (text, '(')) && 
            (func_end_name > 1) && 
            ((int) strlen (text) == kl_util_find_next_bracket (func_end_name - 1, text) + 1))
        {
            // function name
            char* func_name = malloc (strlen (text) + 1);
            strcpy (func_name, text);
            func_name[func_end_name - 1] = '\0';
            func_name = realloc (func_name, func_end_name);
            kl_util_trim (&func_name);

            // function arguments
            char* func_args = malloc (strlen (text) + 1);
            strcpy (func_args, text + func_end_name);
            func_args[strlen (text) - func_end_name - 1] = '\0';
            func_args = realloc (func_args, strlen (text) - func_end_name);
            kl_util_trim (&func_args);

            size_t contains_point;

            if ((kl_lib_get_function_by_name (func_name) != UNKNOWN) ||
                (var_get_pos_by_name (func_name, false) != UNKNOWN) ||
                ((contains_point = kl_util_contains (func_name, ':')) && func_name[contains_point] == ':'))
                    return create_call_function (func_name, func_args, kl_parse_convert_to_value);
        }

        // Lambda
        int func_args_end;
        int func_code_begin, func_code_end;

        if ((text[0] == '(') && 
            ((func_args_end = kl_util_find_next_bracket (0, text)) != -1) && /*func_end_name < strlen (text) && */
            ((func_code_begin = kl_util_contains (text, '{') - 1) != -1) && 
            ((int) strlen (text) == (func_code_end = kl_util_find_next_bracket (func_code_begin, text)) + 1))
        {
            // funcation arguments
            char* func_args = malloc (strlen (text) + 1);
            strcpy (func_args, text + 1);
            func_args[func_args_end - 1] = '\0';
            func_args = realloc (func_args, func_args_end);
            kl_util_trim (&func_args);

            // function code
            char* func_code = malloc (strlen (text) + 1);
            strcpy (func_code, text + func_code_begin + 1);
            func_code[func_code_end - func_code_begin - 1] = '\0';
            func_code = realloc (func_code, func_code_end - func_code_begin);
            kl_util_trim (&func_code);

            // function/lambda name
            static size_t lambda_count = 0;
            lambda_count ++;

            char* lambda_name = malloc (20);
            sprintf (lambda_name, "l%ld", lambda_count);
            lambda_name = realloc (lambda_name, strlen (lambda_name) + 1);

            return create_function (lambda_name, func_args, func_code, kl_parse_convert_to_value);
        }

        // returns value of variable -> ´pointer´
        int var_position;
        if ((var_position = var_get_pos_by_name (text, false)) != -1)
            return POINTER (var_position);
       
        // returns reference/position of variable -> ´int´
        if (text[0] == '$' || text[0] == '~' || text[0] == '!')
        {
            char* tmp_text = malloc (strlen (text));
            strcpy (tmp_text, text + 1);
            kl_util_trim (&tmp_text);
            
            if (text[0] == '!')
            {
                size_t x = kl_intp_register_add (&register_list, REGISTER_ALLOC (kl_parse_convert_to_value (tmp_text)));
                kl_intp_register_add (&register_list, REGISTER_NEG (VALUE_INT (x)));
                return POINTER (x);
            }
            else if (text[0] == '~')
            {
                return POINTER_POINTER (kl_intp_register_add (&register_list, REGISTER_ALLOC (kl_parse_convert_to_value (tmp_text))));
            }
            else if (text[0] == '$' && (var_position = var_get_pos_by_name (tmp_text, false)) != -1)
                return VALUE_INT (var_position);
        }

        int bracket_index_open;
        int bracket_index_close;

        // returns value/pointer using index of string -> ´char´
        if (((bracket_index_open = kl_util_contains (text, '[')) != 0) && 
            ((bracket_index_close = kl_util_find_next_bracket (bracket_index_open - 1, text)) != -1))
        {
            char old_char = text[bracket_index_open - 1];
            text[bracket_index_open - 1] = '\0';
            if ((var_position = var_get_pos_by_name (text, false)) != -1)
            {
                text[bracket_index_open - 1] = old_char;

                text[bracket_index_close] = '\0';
                text += bracket_index_open;
                size_t i = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
                kl_intp_register_add (&register_list, REGISTER_IND (VALUE_INT (i), POINTER (var_position), kl_parse_convert_to_value (text)));
                return POINTER (i);
            }
            text[bracket_index_open - 1] = old_char;
        }

        int var_list_end;
        int var_index_begin, var_index_end;

        if ((text[0] == '(') && 
            ((var_list_end = kl_util_find_next_bracket (0, text)) != -1) && 
            ((var_index_begin = kl_util_contains (text + var_list_end, '[') + var_list_end - 1) != -1) && 
            ((int) strlen (text) == (var_index_end = kl_util_find_next_bracket (var_index_begin, text)) + 1))
        {
            // Variable list
            char* var_list = malloc (strlen (text) + 1);
            strcpy (var_list, text + 1);
            var_list[var_list_end - 1] = '\0';
            var_list = realloc (var_list, var_list_end);

            // Variable index
            char* var_index = malloc (strlen (text) + 1);
            strcpy (var_index, text + var_index_begin + 1);
            var_index[var_index_end - var_index_begin - 1] = '\0';
            var_index = realloc (var_index, var_index_end - var_index_begin);

            size_t i = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
            kl_intp_register_add (&register_list, REGISTER_IND (VALUE_INT (i), kl_parse_convert_to_value (var_list), kl_parse_convert_to_value (var_index)));
            return POINTER (i);
        }

        // Error variable does not exist!
        error ("Variable ´%s´ does not exist in this scope!\n", text);

        return VALUE_INT (0);
    }

    void kl_parse_do_alloc (char* values, bool constant)
    {
        char** args;
        size_t length = kl_util_split (values, ',', &args);
        size_t equal_pos;

        Value  m_value;

        // recieved parameters
        for (size_t i = 0; i < length; ++ i) 
        {
            // check if kl_util_contains an ´=´
            if ((equal_pos = kl_util_contains (args[i], '=')))
            {
                args[i][equal_pos - 1] = '\0';
                m_value = kl_parse_convert_to_value (args[i] + equal_pos); // - 1
                kl_util_trim (&args[i]);
                if (is_illegal_name (args[i]))
                    error ("Variable ´%s´ cannot be called like that!", args[i]);
                var_add (args[i], kl_intp_register_add (&register_list, REGISTER_ALLOC (m_value)), constant);
                continue;
            }
            if (is_illegal_name (args[i]))
                error ("Variable ´%s´ cannot be called like that!", args[i]);
            var_add (args[i], kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))), constant);
        }
    }

    void c_clabel (CmsData* data)
    {
        var_add_function_path ("local", data[0], kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (kl_intp_get_current_register_position (&register_list)))), true);
    }

    void c_set_index (CmsData* data) 
    { 
        kl_intp_register_add (&register_list, REGISTER_IND_SET (POINTER (var_get_pos_by_name (data[0], true)), kl_parse_convert_to_value (data[1]), kl_parse_convert_to_value (data[2])));
    }

    void c_set_index_index (CmsData* data) 
    { 
        kl_intp_register_add (&register_list, REGISTER_IND_IND_SET (POINTER (var_get_pos_by_name (data[0], true)), kl_parse_convert_to_value (data[1]), kl_parse_convert_to_value (data[2]), kl_parse_convert_to_value (data[3])));
    }

    void c_alloc (CmsData* data) { kl_parse_do_alloc (data[0], false); }

    void c_alloc_const (CmsData* data) { kl_parse_do_alloc (data[0], true); }

    void c_jlabel (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_JUMP (kl_parse_convert_to_value (data[0]))); }

    void c_set (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_SET (VALUE_INT (var_get_pos_by_name (data[0], true)), kl_parse_convert_to_value (data[1]))); }

    void c_add (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_ADD (VALUE_INT (var_get_pos_by_name (data[0], true)), kl_parse_convert_to_value (data[1]))); }

    void c_sub (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_SUB (VALUE_INT (var_get_pos_by_name (data[0], true)), kl_parse_convert_to_value (data[1]))); }

    void c_mul (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_MUL (VALUE_INT (var_get_pos_by_name (data[0], true)), kl_parse_convert_to_value (data[1]))); }

    void c_div (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_DIV (VALUE_INT (var_get_pos_by_name (data[0], true)), kl_parse_convert_to_value (data[1]))); }

    void c_set_pointer (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_SET (kl_parse_convert_to_value (data[0]), kl_parse_convert_to_value (data[1]))); }

    void c_add_pointer (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_ADD (kl_parse_convert_to_value (data[0]), kl_parse_convert_to_value (data[1]))); }

    void c_sub_pointer (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_SUB (kl_parse_convert_to_value (data[0]), kl_parse_convert_to_value (data[1]))); }

    void c_mul_pointer (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_MUL (kl_parse_convert_to_value (data[0]), kl_parse_convert_to_value (data[1]))); }

    void c_div_pointer (CmsData* data) { kl_intp_register_add (&register_list, REGISTER_DIV (kl_parse_convert_to_value (data[0]), kl_parse_convert_to_value (data[1]))); }

    void do_list_statements (char* data, Register* (reg)(Value value))
    {
        char** args;
        size_t length = kl_util_split (data, ',', &args);

        for (size_t i = 0; i < length; ++ i)
            kl_intp_register_add (&register_list, reg (kl_parse_convert_to_value (args[i])));

        free (args);
        args = NULL;
    }

    void c_print (CmsData* data)   { do_list_statements (data[0], REGISTER_PRINT); }

    void c_flush (CmsData* data)   { do_list_statements (data[0], REGISTER_FLUSH); }

    void c_input (CmsData* data)   { do_list_statements (data[0], REGISTER_READ); }

    void c_getchar (CmsData* data) { do_list_statements (data[0], REGISTER_READ_CHAR); }

    void c_system (CmsData* data)  { do_list_statements (data[0], REGISTER_SYS); }

    void c_push (CmsData* data) { do_list_statements (data[0], REGISTER_PUSH); }

    void c_pop (CmsData* data) { do_list_statements (data[0], REGISTER_POP); }

    void c_check (CmsData* data)
    {
        Value  m_value = kl_parse_convert_to_value (data[0]);
        size_t m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = kl_intp_register_add (&register_list, REGISTER_NCMP (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        kl_parse_compile (data[1], variables, &variable_count, true);
        register_list[x]->reg_values[2] = VALUE_INT (kl_intp_get_current_register_position (&register_list));
    }

    void c_check_short (CmsData* data)
    {
        strcat (data[1] = realloc (data[1], strlen (data[1]) + 2), ";");
        c_check (data);
    }

    void c_ncheck (CmsData* data)
    {
        Value  m_value = kl_parse_convert_to_value (data[0]);
        size_t m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = kl_intp_register_add (&register_list, REGISTER_CMP (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        kl_parse_compile (data[1], variables, &variable_count, true);
        register_list[x]->reg_values[2] = VALUE_INT (kl_intp_get_current_register_position (&register_list));
    }

    void c_ncheck_short (CmsData* data)
    {
        strcat (data[1] = realloc (data[1], strlen (data[1]) + 2), ";");
        c_ncheck (data);
    }

    void c_loop_indexing (CmsData* data)
    {
        size_t var_index;
        size_t var_len;

        var_add (data[1], var_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))), false);

        var_len = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));

        Value* args_values = malloc (sizeof (Value));
        args_values[0] = kl_parse_convert_to_value (data[0]);

        kl_intp_register_add (&register_list, REGISTER_LIB_FUNC (
            VALUE_INT  (kl_lib_get_function_by_name ("len")),
            VALUE_INT  (var_len),
            VALUE_LIST (args_values, 1)
        ));

        size_t loop_index = kl_intp_get_current_register_position (&register_list);

        size_t x = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
        kl_intp_register_add (&register_list, REGISTER_SMA (POINTER (var_index), POINTER (var_len), VALUE_INT (x)));
        size_t y = kl_intp_register_add (&register_list, REGISTER_CMP (POINTER (x), VALUE_INT (true), VALUE_INT (0)));

        kl_parse_compile (data[2], variables, &variable_count, true);
        kl_intp_register_add (&register_list, REGISTER_ADD  (VALUE_INT (var_index), VALUE_INT (1)));
        kl_intp_register_add (&register_list, REGISTER_JUMP (VALUE_INT (loop_index)));
        register_list[y]->reg_values[2] = VALUE_INT (kl_intp_get_current_register_position (&register_list));

        var_rem ();
    }

    void c_loop_indexing_short (CmsData* data)
    {
        strcat (data[2] = realloc (data[2], strlen (data[2]) + 2), ";");
        c_loop_indexing (data);
    }

    void c_loop_key_val (CmsData* data)
    {
        size_t var_index;
        size_t var_value;
        size_t var_len;

        var_add (data[1], var_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))), false);
        var_add (data[2], var_value = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))), true);

        var_len = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));

        Value* args_values = malloc (sizeof (Value));
        args_values[0] = kl_parse_convert_to_value (data[0]);

        kl_intp_register_add (&register_list, REGISTER_LIB_FUNC (
            VALUE_INT  (kl_lib_get_function_by_name ("len")),
            VALUE_INT  (var_len),
            VALUE_LIST (args_values, 1)
        ));

        size_t loop_index = kl_intp_get_current_register_position (&register_list);

        size_t x = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
        kl_intp_register_add (&register_list, REGISTER_SMA (POINTER (var_index), POINTER (var_len), VALUE_INT (x)));
        size_t y = kl_intp_register_add (&register_list, REGISTER_CMP (POINTER (x), VALUE_INT (true), VALUE_INT (0)));

        kl_intp_register_add (&register_list, REGISTER_IND (VALUE_INT (var_value), args_values[0], POINTER (var_index)));

        kl_parse_compile (data[3], variables, &variable_count, true);
        kl_intp_register_add (&register_list, REGISTER_ADD  (VALUE_INT (var_index), VALUE_INT (1)));
        kl_intp_register_add (&register_list, REGISTER_JUMP (VALUE_INT (loop_index)));
        register_list[y]->reg_values[2] = VALUE_INT (kl_intp_get_current_register_position (&register_list));

        var_rem ();
        var_rem ();
    }

    void c_loop_key_val_short (CmsData* data)
    {
        strcat (data[3] = realloc (data[3], strlen (data[3]) + 2), ";");
        c_loop_key_val (data);
    }


    void c_loop (CmsData* data)
    {
        size_t loop_index = kl_intp_get_current_register_position (&register_list);

        Value  m_value = kl_parse_convert_to_value (data[0]);
        size_t x = kl_intp_register_add (&register_list, REGISTER_NCMP (m_value, VALUE_INT (false), VALUE_INT (0)));
        kl_parse_compile (data[1], variables, &variable_count, true);
        kl_intp_register_add (&register_list, REGISTER_JUMP (VALUE_INT (loop_index)));
        register_list[x]->reg_values[2] = VALUE_INT (kl_intp_get_current_register_position (&register_list));
    }

    void c_loop_short (CmsData* data)
    {
        strcat (data[1] = realloc (data[1], strlen (data[1]) + 2), ";");
        c_loop (data);
    }

    void c_check_else (CmsData* data)
    {
        Value  m_value = kl_parse_convert_to_value (data[0]);
        size_t m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = kl_intp_register_add (&register_list, REGISTER_NCMP (POINTER (m_index), VALUE_INT (false), VALUE_INT (0)));
        {
            kl_parse_compile (data[1], variables, &variable_count, true);
        }
        size_t y = kl_intp_register_add (&register_list, REGISTER_JUMP (VALUE_INT (0)));
        register_list[x]->reg_values[2] = VALUE_INT (kl_intp_get_current_register_position (&register_list));
        {
            kl_parse_compile (data[2], variables, &variable_count, true);
        }
        register_list[y]->reg_values[0] = VALUE_INT (kl_intp_get_current_register_position (&register_list));
    }

    void c_ncheck_else (CmsData* data)
    {
        Value  m_value = kl_parse_convert_to_value (data[0]);
        size_t m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = kl_intp_register_add (&register_list, REGISTER_CMP (POINTER (m_index), VALUE_INT (false), VALUE_INT (0)));
        {
            kl_parse_compile (data[1], variables, &variable_count, true);
        }
        size_t y = kl_intp_register_add (&register_list, REGISTER_JUMP (VALUE_INT (0)));
        register_list[x]->reg_values[2] = VALUE_INT (kl_intp_get_current_register_position (&register_list));
        {
            kl_parse_compile (data[2], variables, &variable_count, true);
        }
        register_list[y]->reg_values[0] = VALUE_INT (kl_intp_get_current_register_position (&register_list));
    }

    void c_function (CmsData* data) { create_function (data[0], data[1], data[2], kl_parse_convert_to_value); }

    void c_function_short (CmsData* data)
    {
        strcat (data[2] = realloc (data[2], strlen (data[2]) + 2), ";");
        create_function (data[0], data[1], data[2], kl_parse_convert_to_value);
    }

    void c_function_ret (CmsData* data)
    {
        char* f_code = malloc (4 + strlen (data[2]) + 1 + 1);
        sprintf (f_code, "ret %s;", (char*) data[2]);
        create_function (data[0], data[1], f_code, kl_parse_convert_to_value);
        free (f_code);
        f_code = NULL;
    }

    void c_call (CmsData* data) { create_call_function (data[0], data[1], kl_parse_convert_to_value); }

    void c_return (CmsData* data)
    {
        if (in_function > 0) // function
        {
            if (data != NULL) kl_intp_register_add (&register_list, REGISTER_SET (VALUE_INT (in_function + 3), kl_parse_convert_to_value (data[0])));
            kl_intp_register_add (&register_list, REGISTER_JUMP (POINTER (in_function + 2)));
        }
        else if (in_function == 0) // main
        {
            kl_intp_register_add (&register_list, REGISTER_JUMP (VALUE_INT (-1)));
        }
        else // scope
        {
            if (scope_jump_back_size >= 100)
                error ("Maximum ´ret´ amount in ´scope´ of 100 has been reached!", NULL);
            if (data != NULL) kl_intp_register_add (&register_list, REGISTER_SET (VALUE_INT (abs(in_function)), kl_parse_convert_to_value (data[0])));
            scope_jump_back[scope_jump_back_size ++] = kl_intp_register_add (&register_list, REGISTER_JUMP (VALUE_INT (-1)));
        }
    }

    void c_scope (CmsData* data)
    {
        int m_index = kl_intp_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))); // alloc memory for return
        int old_in_function = in_function;
        in_function = -m_index; // store old ´in_function´ and set new one (neg used for ´scope´ in ret)

        kl_parse_compile (data[0], variables, &variable_count, true); // compile code in bracket

        Value value_current_reg_pos = VALUE_INT (kl_intp_get_current_register_position (&register_list)); 
        for (byte i = 0; i < scope_jump_back_size; ++ i)
            register_list[scope_jump_back[i]]->reg_values[0] = value_current_reg_pos;
        scope_jump_back_size = 0;

        in_function = old_in_function; // restore old ´in_function´
    }

    void c_include (CmsData* data)
    {
        int old_line = cms_get_current_line_number ();
        cms_set_current_line_number (0);

        kl_util_trim_front_end ((char**)(&data[0]));

        kl_parse_compile (kl_util_read_file (data[0]), variables, &variable_count, false);
        cms_set_current_line_number (old_line);
    }

    void c_unref_warning (__attribute__((unused)) CmsData* data)
    {
        warning ("Missing Brackets around statement. Statement will be ignored.\n", NULL);
    }

    void sub_fstream (Value value, char* code)
    {
        kl_util_trim (&code);

        int start_args = kl_util_contains (code, '(');

        if (!start_args)
            return;

        int end_args = kl_util_find_next_bracket (start_args - 1, code);

        char* fargs = kl_util_substr (code, start_args, end_args);
        char* _fname = kl_util_substr (code, 0, start_args - 1);

        kl_util_trim (&_fname);

        char* fname = malloc (strlen (_fname) + 2 + 1);
        
        sprintf (fname, "::%s", _fname);

        Value ret_value = create_call_function_value (fname, value, fargs, kl_parse_convert_to_value);

        free (_fname);
        free (fname);
        free (fargs);

        int start_new_stream = kl_util_contains (code, ':');
        
        if (start_new_stream && code[start_new_stream] == ':')
            sub_fstream (ret_value, code + start_new_stream + 1);
    }

    void c_fstream (CmsData* data)
    {
        sub_fstream (kl_parse_convert_to_value (data[0]), data[1]);
    }

    // var & val getting recocnised even if char is with out space next to it!
    cms_create ( &cms_template, CMS_LIST ( {
        cms_add ("inc %;",              c_include,      CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING_LENGTH | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("var %;",              c_alloc,        CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING_LENGTH | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("val %;",              c_alloc_const,  CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING_LENGTH | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("ret %;",              c_return,       CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING_LENGTH | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("ret ;",               c_return,       CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING        | CMS_USE_BRACKET_SEARCH_ALGORITHM);

        cms_add ("{ % }",               c_scope,        CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);

        cms_add ("( % ), (#, #) -> { % }",  c_loop_key_val,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("#, (#, #) -> { % }",      c_loop_key_val,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ), (#, #) -> % ;",    c_loop_key_val_short,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("#, (#, #) -> % ;",        c_loop_key_val_short,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);

        cms_add ("( % ), # -> { % }",   c_loop_indexing,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("#, # -> { % }",       c_loop_indexing,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ), # -> % ;",     c_loop_indexing_short,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("#, # -> % ;",         c_loop_indexing_short,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);

        cms_add ("( % ) :: % ;",        c_fstream,      CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# :: % ;",            c_fstream,      CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);

        cms_add ("# ( % ) { % }",       c_function,     CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# ( % ) > % ;",       c_function_short,    CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# ( % ) < % ;",       c_function_ret, CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# ( % ) ;",           c_call,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) ( % ) ;",       c_call,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);

        cms_add ("~ # += % ;",          c_add_pointer,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # -= % ;",          c_sub_pointer,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # *= % ;",          c_mul_pointer,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # /= % ;",          c_div_pointer,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # = % ;",           c_set_pointer,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# += % ;",            c_add,          CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# -= % ;",            c_sub,          CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# *= % ;",            c_mul,          CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# /= % ;",            c_div,          CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# = % ;",             c_set,          CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# [ % ] [ % ] = % ;", c_set_index_index,    CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# [ % ] = % ;",       c_set_index,    CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);

        cms_add ("O: % ;",              c_print,        CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("I: % ;",              c_input,        CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("O ( flush ) : % ;",   c_flush,        CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("I ( single ) : % ;",  c_getchar,      CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("S: % ;",              c_system,       CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("PUSH: % ;",           c_push,         CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("POP: % ;",            c_pop,          CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);

        cms_add ("jump % ;",            c_jlabel,       CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("#:",                  c_clabel,       CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);

        cms_add ("! ( % ) { % } { % }", c_ncheck_else,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! ( % ) { % }",       c_ncheck,       CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! ( % ) > % ;",       c_ncheck_short, CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! # { % } { % }",     c_ncheck_else,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! # { % }",           c_ncheck,       CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! # > % ;",           c_ncheck_short, CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) { % } { % }",   c_check_else,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) { % }",         c_check,        CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) > % ;",         c_check_short,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # { % } { % }",     c_unref_warning,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # { % }",           c_unref_warning,        CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # > % ;",           c_unref_warning,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# { % } { % }",       c_check_else,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# { % }",             c_check,        CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# > % ;",             c_check_short,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);

        cms_add ("~ # -> { % }",        c_unref_warning,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("~ # -> % ;",          c_unref_warning,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) -> { % }",      c_loop,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# -> { % }",          c_loop,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) -> % ;",        c_loop_short,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# -> % ;",            c_loop_short,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
    } ));

    // Search syntax using ´cms_template´ in ´example_text´
    cms_find (code, cms_template);

    if (reset_variables)
        (*variables) = realloc (*variables, sizeof (Variable) * (*pre_variable_count));
    else
        (*pre_variable_count) = variable_count;

    return EXIT_SUCCESS;
}
