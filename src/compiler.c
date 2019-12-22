#include "compiler.h"
#include "multisearcher.h"
#include "gnumber.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

void error (const char* msg, const void* variablen, ...)
{
    int line_number = cms_get_current_line_number ();
    int nDigits = floor (log10 (abs (line_number))) + 1;

    char* arrow = malloc (nDigits * 3 + 1);
    size_t i;

    *arrow = '\0';
    for (i = 0; i < nDigits; ++ i)
        strcat (arrow, "─");
    arrow[i * 3 + 1] = '\0';

    char* line = cms_get_current_line ();
    frs_trim (&line);

    fprintf (stderr, "\x1b[90m[\x1b[33m%d\x1b[90m]\x1b[92m─►\x1b[93m %s\n \x1b[92m└%s─► \x1b[91m(ERR) ", line_number, line, arrow);
    fprintf (stderr, msg, variablen);
    fprintf (stderr, "\x1b[0m\n");

    free (arrow);

    exit (EXIT_SUCCESS);
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

// in_function > 0 then fr_compile is in a ´function´
// in_function = 0 then fr_compile is in ´main´
// in_function < 0 then fr_compile is in ´scope´
int in_function = 0;

// Used to set variable name;
char* function_path = NULL;

// scope, ret back jump position stored in array to set later -> TODO: fix bug if multiple ´scope´ ret
size_t scope_jump_back[100];
size_t scope_jump_back_size = 0;


// Here the code will be compiled into a list of registers -> ´register_list´
int fr_compile (char* code, Variable** variables, size_t* pre_variable_count, const bool reset_variables)
{
    if (!code || !variables)
        return EXIT_SUCCESS;

    // Filter comments out of code -> TODO: Move comment out of ´fr_compile´ -> performance
    frs_filter_comment (&code);

    // Add ´local.´ to function_path if ´fr_compile´ is main
    if (!function_path)
        strcpy (function_path = malloc (strlen ("local") + 1), "local");

    // Used to reset ´variables´ back to normal size at the end of ´fr_compile´
    size_t variable_count = *pre_variable_count;

    // Template storing syntax, used for the CMultiSearcher
    CmsTemplate* cms_template;

    // Returns the index of variable by given name
    int var_get_pos_by_name (char* name, bool wants_to_modify)
    {
        frs_ctrim (&name);

        Variable variable;
        char* full_var_name;
        int i;

        // Variable has full varname path
        for (i = variable_count - 1; i >= 0; -- i)
        {
            full_var_name = malloc (strlen ((*variables)[i].function_path) + strlen ((*variables)[i].name) + 2);
            sprintf (full_var_name, "%s.%s", (*variables)[i].function_path, (*variables)[i].name);

            bool exist = !strcmp (full_var_name, name);

            free (full_var_name);

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

        // Variable has full varname path beginning from ´local.´
        for (i = variable_count - 1; i >= 0; -- i)
        {
            full_var_name = malloc (strlen ((*variables)[i].function_path) + strlen ((*variables)[i].name) + 2);
            sprintf (full_var_name, "%s.%s", (*variables)[i].function_path, (*variables)[i].name);

            bool exist = !strcmp (full_var_name + strlen ("local") + 1, name);

            free (full_var_name);

            if (!exist)
                continue;

            variable = (*variables)[i];
            goto var_get_pos_by_name_return;
        }

        return -1;

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

    size_t var_add (const char* name, const size_t m_index, const bool constant)
    {
        var_add_function_path (function_path, name, m_index, constant);
    }

    // Doesn't work with ´\´ and ´\\´
    // Used for special string inserts example: "${}" or "$VARIABLE"
    Value create_filled_in_str (char* text, Value (fr_convert_to_value) (char* text))
    {
        text = frs_str_replace (frs_str_replace (frs_str_replace (frs_str_replace (frs_str_replace (frs_str_replace (frs_str_replace (text, "\\\\", "$/638$"), "\\\"", "\""), "\\r", "\r"), "\\t", "\t"), "\\x1b", "\x1b"), "\\n", "\n"), "$/638$", "\\");

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
                int last_bracket = frs_find_next_bracket (++ i, text);

                if (!has_variables)
                {
                    m_index = fr_register_add (&register_list, REGISTER_ALLOC (VALUE_STR ("")));
                    has_variables = 1;
                }

                text[i - 1] = '\0';
                text[i] = '\0';

                fr_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), VALUE_STR (text)));
                fr_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), fr_convert_to_value (frs_substr (text+i+1, 0, last_bracket - i - 1))));

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
                    tvar = frs_substr (text, i + 1, i + 1 + strlen (cvar));
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
                fr_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), POINTER (var_get_pos_by_name (varname, false))));

                text += i + 1 + strlen (varname);
                i = -1;
            }
        }

        if (has_variables)
        {
            if (text[0] != '\0')
                fr_register_add (&register_list, REGISTER_ADD (VALUE_INT (m_index), VALUE_STR (text)));
            return POINTER (m_index);
        }
        return VALUE_STR (text);
    }


    // WRONG POSITION FOR ARGUMENTS
    Value create_call_function (char* func_name, char* func_args, Value (fr_convert_to_value) (char* text))
    {
        // printf (">>> %s ( %s ) ;\n", func_name, func_args);

        char** args;
        size_t length = 0; 

        if (func_args != NULL && func_args[0] != '\0')
            length = frs_split (func_args, ',', &args);

        Value position = fr_convert_to_value (func_name); 

        size_t m_tmp_var;

        int func_args_positions[length];
        int func_args_values[length];

        // Arguments passed over call
        for (size_t i = 0; i < length; ++ i) 
        {
            func_args_positions[i] = m_tmp_var = fr_register_add (&register_list, REGISTER_ALLOC (position));
            fr_register_add (&register_list, REGISTER_SUB (VALUE_INT (m_tmp_var), VALUE_INT (i + 4)));
            fr_register_add (&register_list, REGISTER_PUSH (POINTER_POINTER (m_tmp_var))); // pushes variable to stack, used later to reset variable
            func_args_values[i] = fr_register_add (&register_list, REGISTER_ALLOC (fr_convert_to_value (args[i])));
        }

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_SET (POINTER (func_args_positions[i]), POINTER (func_args_values[i])));

        // Argument for location of call ´__origin__´
        size_t m_tmp = fr_register_add (&register_list, REGISTER_ALLOC (position));
        fr_register_add (&register_list, REGISTER_SUB (VALUE_INT (m_tmp), VALUE_INT (3)));
        fr_register_add (&register_list, REGISTER_PUSH (POINTER_POINTER (m_tmp))); // pushes variable to stack, used later to reset variable
        fr_register_add (&register_list, REGISTER_SET (POINTER (m_tmp), VALUE_INT (fr_get_current_register_position (&register_list) + 2)));

        // Calling jump
        fr_register_add (&register_list, REGISTER_JUMP (position));

        // pops old variable __origin__ from stack 
        fr_register_add (&register_list, REGISTER_POP (POINTER (m_tmp)));

        // pops variable to stack, used later to reset variable
        for (int i = length - 1; i >= 0; -- i) 
            fr_register_add (&register_list, REGISTER_POP (POINTER (func_args_positions[i])));

        // Return value
        m_tmp_var = fr_register_add (&register_list, REGISTER_ALLOC (position));
        m_tmp_var = fr_register_add (&register_list, REGISTER_ALLOC (POINTER_POINTER (m_tmp_var)));

        return POINTER (m_tmp_var);
    }

    Value create_function (char* func_name, char* func_args, char* func_code, Value (fr_convert_to_value) (char* text))
    {
        char** args;
        char*  var_name;

        // argument length
        size_t length = 0;

        char* old_function_path;
        strcpy (old_function_path = malloc (strlen (function_path) + 1), function_path);

        // Add to function_path
        if (func_name != NULL)
        {
            strcat (function_path = realloc (function_path, strlen (function_path) + strlen (func_name) + 2), ".");
            strcat (function_path, func_name);
        }
        
        // frs_split arguments of function
        if (func_args[0] != '\0')
            length = frs_split (func_args, ',', &args);

        Value  m_value;

        size_t equal_pos;
        size_t func_pos;

        // recieved parameters
        for (int i = length - 1; i >= 0; -- i) 
        {
            // check if frs_contains an ´=´
            if (equal_pos = frs_contains (args[i], '='))
            {
                args[i][equal_pos - 1] = '\0';
                m_value = fr_convert_to_value (args[i] + equal_pos); // - 1
                frs_ctrim (&args[i]);
                var_add (args[i], fr_register_add (&register_list, REGISTER_ALLOC (m_value)), false);
            }
            else
            {
                var_add (args[i], fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))), false);
            }
        }

        // first default argument, ´__origin__´ used to determine where to go back
        sprintf (var_name = malloc (10 + 1), "__origin__");
        var_add (var_name, fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (-1))), false);


        // Allocate memory for function position
        
        Value value_funk_position = VALUE_INT (fr_get_current_register_position (&register_list) + 2);
        func_pos = fr_register_add (&register_list, REGISTER_ALLOC (value_funk_position));
        if (func_name != NULL)
            var_add_function_path (old_function_path, func_name, func_pos, false);

        // Jump to end of functions body, will not happen if function is called
        size_t x = fr_register_add (&register_list, REGISTER_JUMP (VALUE_INT (-1)));

        // Allocate memory for function return
        fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));

        // Compile function body
        int old_in_function = in_function;
        in_function = func_pos;
        fr_compile (func_code, variables, &variable_count, true);
        in_function = old_in_function;
         
        // Jump to call position (location of call saved in ´__origin__´)
        fr_register_add (&register_list, REGISTER_JUMP (fr_convert_to_value (var_name)));

        // Set skip jump to current location of register
        register_list[x]->reg_values[0] = VALUE_INT (fr_get_current_register_position(&register_list));
 
        // Remove from function_path
        if (func_name != NULL)
        {
            size_t function_path_size = strlen (function_path) - strlen (func_name) - 1;
            function_path[function_path_size] = '\0';
            function_path = realloc (function_path, function_path_size + 1);
        }

        return value_funk_position;
    }

    // Converts a string to ´Value´ supports also different types of Values
    Value fr_convert_to_value (char* text)
    {
        // Trim text begin & end
        frs_trim (&text);
    

        // Remove brackets if outside is bracket
        if (text[0] == '(' && strlen (text) == frs_find_next_bracket (0, text) + 1)
            text[strlen (text ++) - 1] = '\0'; // remove start & end bracket

        // scope
        if (text[0] == '{' && strlen (text) == frs_find_next_bracket (0, text) + 1)
        {
            text[strlen (text ++) - 1] = '\0'; // remove start & end bracket
            int m_index = fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))); // alloc memory for return
            int old_in_function = in_function;
            in_function = -m_index; // store old ´in_function´ and set new one (neg used for ´scope´ in ret)

            fr_compile (text, variables, &variable_count, true); // compile code in bracket

            Value value_current_reg_pos = VALUE_INT (fr_get_current_register_position (&register_list)); 
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

        size_t is_if   = frs_contains (text, '?');
        size_t is_else = frs_contains (text, ':');

        // short if 
        if (is_if && is_else)
        {
            text[is_if - 1] = '\0';    // set '\0' at '?'
            text[is_else  - 1] = '\0'; // set '\0' at ':'
            Value val_condition = fr_convert_to_value (text); // create value for condition

            size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (val_condition)); // alloc memory for return

            size_t x = fr_register_add (&register_list, REGISTER_NCMP (POINTER (m_index), VALUE_INT (0), VALUE_INT (0))); // condition if
            {
                fr_register_add (&register_list, REGISTER_SET (VALUE_INT (m_index), fr_convert_to_value (text += is_if))); // set memory value if true
            }
            size_t y = fr_register_add (&register_list, REGISTER_JUMP (VALUE_INT (0))); // condition else
            register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list)); // set ´condition if´ jump point
            {
                fr_register_add (&register_list, REGISTER_SET (VALUE_INT (m_index), fr_convert_to_value (text += is_else - is_if))); // set memory value if false
            }
            register_list[y]->reg_values[0] = VALUE_INT (fr_get_current_register_position (&register_list)); // set ´condition else´ jump point

            return POINTER (m_index); // return pointer of memory
        }

        size_t is_bigger  = frs_contains (text, '>'); // >,  >=
        size_t is_smaller = frs_contains (text, '<'); // <,  <=
        size_t is_equal   = frs_contains (text, '='); // ==, !=

        char* v1_text; // condition before compute
        char* v2_text; // condition after  compute

        if (is_bigger)
        {
            strcpy (v1_text = malloc (is_bigger), text);
            v1_text[is_bigger - 1] = '\0';

            strcpy (v2_text = malloc (strlen (text) - is_bigger), text + is_bigger + (text[is_bigger] == '=' ? 1 : 0));

            int m_index = fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
            fr_register_add (&register_list, (text[is_bigger] == '=' ? REGISTER_BEQ : REGISTER_BIG) (fr_convert_to_value (v1_text), fr_convert_to_value (v2_text), VALUE_INT (m_index)));
            return POINTER (m_index);
        }
        else if (is_smaller)
        {
            strcpy (v1_text = malloc (is_smaller), text);
            v1_text[is_smaller - 1] = '\0';

            strcpy (v2_text = malloc (strlen (text) - is_smaller), text + is_smaller + (text[is_smaller] == '=' ? 1 : 0));

            int m_index = fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
            fr_register_add (&register_list, (text[is_smaller] == '=' ? REGISTER_SEQ : REGISTER_SMA) (fr_convert_to_value (v1_text), fr_convert_to_value (v2_text), VALUE_INT (m_index)));
            return POINTER (m_index);
        }
        else if (is_equal && (text[is_equal] == '=' || text[(is_equal -= 1) - 1] == '!'))
        {
            strcpy (v1_text = malloc (is_equal), text);
            v1_text[is_equal - 1] = '\0';
            strcpy (v2_text = malloc (strlen (text) - is_equal), text + is_equal + 1);

            int m_index = fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
            fr_register_add (&register_list, (text[is_equal-1] == '=' ? REGISTER_EQ : REGISTER_NEQ) (fr_convert_to_value (v1_text), fr_convert_to_value (v2_text), VALUE_INT (m_index)));
            return POINTER (m_index);
        }

        if (!frs_is_str_concat (text)) // ´text´ is a ´string´
        {
            text[strlen (text ++) - 1] = '\0'; // Remove last ´"´
            return create_filled_in_str(text, fr_convert_to_value);
        }
        else if ((text[0] == '\'' && text[strlen (text) - 1] == '\'') && strlen (text) <= 6) // ´text´ is a ´char´
        {
            text[strlen (text ++) - 1] = '\0'; // Remove last ´'´
            return VALUE_CHAR (frs_str_replace (frs_str_replace (frs_str_replace (frs_str_replace (frs_str_replace (frs_str_replace (frs_str_replace (frs_str_replace (text, "\\\\", "$/638$"), "\\'", "'"), "\\r", "\r"), "\\t", "\t"), "\\x1b", "\x1b"), "\\n", "\n"), "$/638$", "\\"), "\\'", "'")[0]);
        }

        // Check if text is a ´int´ or ´float´
        bool var_is_number = true;
        bool var_is_float  = true;

        for (size_t i = 0; text[i] != '\0'; ++ i)
        {
            if ((text[i] == '-' || text[i] == '+') && i == 0) // skip if first char is + or -
                continue;

            if (text[i] < '0' || text[i] > '9')
                var_is_number = false;
            if ((text[i] < '0' || text[i] > '9') && text[i] != '.')
                var_is_float = false;
        }

        // ´text´ is ´int´
        if (var_is_number)
            return VALUE_INT (atoi (text));

        // ´text´ is ´float´
        if (var_is_float)
            return VALUE_FLOAT (atof (text));

        // calculation
        if (frs_contains (text, '+') || frs_contains (text, '-') || frs_contains (text, '*') || frs_contains (text, '/') || frs_contains (text, '%'))
            return gna_registry_calculation_simple (&register_list, text, fr_convert_to_value);

        // Function call
        size_t func_end_name = 0;

        if ((func_end_name = frs_contains (text, '(')) && func_end_name > 1 && strlen (text) == frs_find_next_bracket (func_end_name - 1, text) + 1)
        {
            char* func_name = malloc (strlen (text) + 1);
            strcpy (func_name, text);
            func_name[func_end_name - 1] = '\0';
            func_name = realloc (func_name, func_end_name - 1);
            frs_trim (&func_name);

            char* func_args = malloc (strlen (text) + 1);
            strcpy (func_args, text + func_end_name);
            func_args[strlen (text) - func_end_name - 1] = '\0';
            func_args = realloc (func_args, strlen (text) - func_end_name);
            frs_trim (&func_args);

            // Check if function exist
            if (var_get_pos_by_name (func_name, false) != -1)
                return create_call_function (func_name, func_args, fr_convert_to_value);
        }

        // Lambda
        
        size_t func_args_end;
        size_t func_code_begin, func_code_end;

        if (text[0] == '(' && (func_args_end = frs_find_next_bracket (0, text)) != -1 && func_end_name < strlen (text) && 
                (func_code_begin = frs_contains (text, '{') - 1) != -1 && strlen (text) == (func_code_end = frs_find_next_bracket (func_code_begin, text)) + 1)
        {
            char* func_args = malloc (strlen (text) + 1);
            strcpy (func_args, text + 1);
            func_args[func_args_end - 1] = '\0';
            func_args = realloc (func_args, func_args_end);
            frs_trim (&func_args);

            char* func_code = malloc (strlen (text) + 1);
            strcpy (func_code, text + func_code_begin + 1);
            func_code[func_code_end - func_code_begin - 1] = '\0';
            func_code = realloc (func_code, func_code_end - func_code_begin);
            frs_trim (&func_code);

            return create_function (NULL, func_args, func_code, fr_convert_to_value);
        }


        // returns value of variable -> ´pointer´
        int var_position;
        if ((var_position = var_get_pos_by_name (text, false)) != -1)
            return POINTER (var_position);
       
        // returns reference/position of variable -> ´int´
        if (text[0] == '&' || text[0] == '~' || text[0] == '!')
        {
            char* tmp_text = malloc (strlen (text));
            strcpy (tmp_text, text + 1);
        
            frs_trim (&tmp_text);
            
            if (text[0] == '!')
            {
                size_t x = fr_register_add (&register_list, REGISTER_ALLOC (fr_convert_to_value (tmp_text)));
                fr_register_add (&register_list, REGISTER_NEG (VALUE_INT (x)));
                return POINTER (x);
            }
            else if (text[0] == '~')
                return POINTER_POINTER (fr_register_add (&register_list, REGISTER_ALLOC (fr_convert_to_value (tmp_text))));
            else if (text[0] == '&' && (var_position = var_get_pos_by_name (tmp_text, false)) != -1)
                return VALUE_INT (var_position);
        }

        size_t bracket_index_open;
        size_t bracket_index_close;

        // returns value/pointer using index of string -> ´char´
        if ((bracket_index_open = frs_contains (text, '[')) != -1 && (bracket_index_close = frs_find_next_bracket (bracket_index_open - 1, text)) != -1)
        {
            char old_char = text[bracket_index_open - 1] = '\0';
            if ((var_position = var_get_pos_by_name (text, false)) == -1)
                error ("Variable ´%s´ does not exist in this scope!\n", text);
            text[bracket_index_open - 1] = old_char;

            text[bracket_index_close] = '\0';
            text += bracket_index_open;
            size_t i = fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));
            fr_register_add (&register_list, REGISTER_IND (VALUE_INT (i), POINTER (var_position), fr_convert_to_value (text)));
            return POINTER (i);
        }

        // Error variable does not exist!
        error ("Variable ´%s´ does not exist in this scope!\n", text);
    }

    void fr_do_alloc (char* values, bool constant)
    {
        char** args;
        size_t length = frs_split (values, ',', &args);
        size_t equal_pos;

        Value  m_value;

        // recieved parameters
        for (int i = 0; i < length; ++ i) 
        {
            // check if frs_contains an ´=´
            if (equal_pos = frs_contains (args[i], '='))
            {
                args[i][equal_pos - 1] = '\0';
                m_value = fr_convert_to_value (args[i] + equal_pos); // - 1
                frs_trim (&args[i]);
                var_add (args[i], fr_register_add (&register_list, REGISTER_ALLOC (m_value)), constant);
                continue;
            }
            var_add (args[i], fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))), constant);
        }
    }

    void c_alloc (CmsData* data, int size)
    {
        fr_do_alloc (data[0], false);
    }

    void c_alloc_const (CmsData* data, int size)
    {
        fr_do_alloc (data[0], true);
    }

    void c_clabel (CmsData* data, int size)
    {
        var_add_function_path ("local", data[0], fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (fr_get_current_register_position (&register_list)))), true);
    }

    void c_jlabel (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_JUMP (fr_convert_to_value (data[0])));
    }

    void c_set (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_SET (VALUE_INT (var_get_pos_by_name (data[0], true)), fr_convert_to_value (data[1])));
    }

    void c_add (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_ADD (VALUE_INT (var_get_pos_by_name (data[0], true)), fr_convert_to_value (data[1])));
    }

    void c_sub (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_SUB (VALUE_INT (var_get_pos_by_name (data[0], true)), fr_convert_to_value (data[1])));
    }

    void c_mul (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_MUL (VALUE_INT (var_get_pos_by_name (data[0], true)), fr_convert_to_value (data[1])));
    }

    void c_div (CmsData* data, int size)
    {
        fr_register_add (&register_list, REGISTER_DIV (VALUE_INT (var_get_pos_by_name (data[0], true)), fr_convert_to_value (data[1])));
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

        size_t length = frs_split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_OUT (fr_convert_to_value (args[i])));

        free (args);
    }

    void c_input (CmsData* data, int size)
    {
        char** args;

        size_t length = frs_split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_CIN (fr_convert_to_value (args[i])));

        free (args);
    }

    void c_getchar (CmsData* data, int size)
    {
        char** args;

        size_t length = frs_split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_GCH (fr_convert_to_value (args[i])));

        free (args);
    }


    void c_system (CmsData* data, int size)
    {
        char** args;

        size_t length = frs_split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_SYS (fr_convert_to_value (args[i])));

        free (args);
    }

    void c_push (CmsData* data, int size)
    {
        char** args;

        size_t length = frs_split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_PUSH (fr_convert_to_value (args[i])));

        free (args);
    }

    void c_pop (CmsData* data, int size)
    {
        char** args;

        size_t length = frs_split (data[0], ',', &args);

        for (size_t i = 0; i < length; ++ i)
            fr_register_add (&register_list, REGISTER_POP (fr_convert_to_value (args[i])));

        free (args);
    }

    void c_check (CmsData* data, int size)
    {
        Value  m_value = fr_convert_to_value (data[0]);
        size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = fr_register_add (&register_list, REGISTER_NCMP (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        fr_compile (data[1], variables, &variable_count, true);
        register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list));
    }

    void c_check_short (CmsData* data, int size)
    {
        sprintf (data[1] = realloc (data[1], strlen (data[1]) + 2), "%s;", data[1]);
        c_check (data, size);
    }


    void c_ncheck (CmsData* data, int size)
    {
        Value  m_value = fr_convert_to_value (data[0]);
        size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = fr_register_add (&register_list, REGISTER_CMP (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        fr_compile (data[1], variables, &variable_count, true);
        register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list));
    }

    void c_ncheck_short (CmsData* data, int size)
    {
        sprintf (data[1] = realloc (data[1], strlen (data[1]) + 2), "%s;", data[1]);
        c_ncheck (data, size);
    }

    void c_loop (CmsData* data, int size)
    {
        size_t loop_index = fr_get_current_register_position (&register_list);

        Value  m_value = fr_convert_to_value (data[0]);
        size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = fr_register_add (&register_list, REGISTER_NCMP (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        fr_compile (data[1], variables, &variable_count, true);
        fr_register_add (&register_list, REGISTER_JUMP (VALUE_INT (loop_index)));
        register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list));
    }

    void c_loop_short (CmsData* data, int size)
    {
        sprintf (data[1] = realloc (data[1], strlen (data[1]) + 2), "%s;", data[1]);
        c_loop (data, size);
    }

    void c_check_else (CmsData* data, int size)
    {
        Value  m_value = fr_convert_to_value (data[0]);
        size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = fr_register_add (&register_list, REGISTER_NCMP (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        {
            fr_compile (data[1], variables, &variable_count, true);
        }
        size_t y = fr_register_add (&register_list, REGISTER_JUMP (VALUE_INT (0)));
        register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list));
        {
            fr_compile (data[2], variables, &variable_count, true);
        }
        register_list[y]->reg_values[0] = VALUE_INT (fr_get_current_register_position (&register_list));
    }

    void c_ncheck_else (CmsData* data, int size)
    {
        Value  m_value = fr_convert_to_value (data[0]);
        size_t m_index = fr_register_add (&register_list, REGISTER_ALLOC (m_value));

        size_t x = fr_register_add (&register_list, REGISTER_CMP (POINTER (m_index), VALUE_INT (0), VALUE_INT (0)));
        {
            fr_compile (data[1], variables, &variable_count, true);
        }
        size_t y = fr_register_add (&register_list, REGISTER_JUMP (VALUE_INT (0)));
        register_list[x]->reg_values[2] = VALUE_INT (fr_get_current_register_position (&register_list));
        {
            fr_compile (data[2], variables, &variable_count, true);
        }
        register_list[y]->reg_values[0] = VALUE_INT (fr_get_current_register_position (&register_list));
    }

    void c_function (CmsData* data, int size)
    {
        create_function (data[0], data[1], data[2], fr_convert_to_value);
    }

    void c_function_short (CmsData* data, int size)
    {
        sprintf (data[2] = realloc (data[2], strlen (data[2]) + 2), "%s;", data[2]);
        create_function (data[0], data[1], data[2], fr_convert_to_value);
    }

    void c_call (CmsData* data, int size)
    {
        create_call_function (data[0], data[1], fr_convert_to_value);
    }

    void c_return (CmsData* data, int size)
    {
        if (in_function > 0) // function
        {
            fr_register_add (&register_list, REGISTER_SET (VALUE_INT (in_function + 2), fr_convert_to_value (data[0])));
            fr_register_add (&register_list, REGISTER_JUMP (POINTER (in_function - 1)));
        }
        else if (in_function == 0) // main
        {
            fr_register_add (&register_list, REGISTER_JUMP (VALUE_INT (-1)));
        }
        else // scope
        {
            if (scope_jump_back_size >= 100)
                error ("Maximum ´ret´ amount in ´scope´ of 100 has been reached!", NULL);
            fr_register_add (&register_list, REGISTER_SET (VALUE_INT (abs(in_function)), fr_convert_to_value (data[0])));
            scope_jump_back[scope_jump_back_size ++] = fr_register_add (&register_list, REGISTER_JUMP (VALUE_INT (-1)));
        }
    }

    void c_scope (CmsData* data, int size)
    {
//        fr_compile (data[0], variables, variable_count);
        int m_index = fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0))); // alloc memory for return
        int old_in_function = in_function;
        in_function = -m_index; // store old ´in_function´ and set new one (neg used for ´scope´ in ret)

        fr_compile (data[0], variables, &variable_count, true); // compile code in bracket

        Value value_current_reg_pos = VALUE_INT (fr_get_current_register_position (&register_list)); 
        for (byte i = 0; i < scope_jump_back_size; ++ i)
            register_list[scope_jump_back[i]]->reg_values[0] = value_current_reg_pos;
        scope_jump_back_size = 0;

        in_function = old_in_function; // restore old ´in_function´
    }

    void c_include (CmsData* data, int size)
    {
        ((char*)data[0])[strlen (data[0] ++) - 1] = '\0'; // remove start & end bracket
        fr_compile (frs_read_file (data[0]), variables, &variable_count, false);
    }

    // Check if c_check_else & c_check can be replaced with c_bracket...
    cms_create ( &cms_template, CMS_LIST ( {
        cms_add ("{ % }",                  c_scope,             CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# ( % ) { % }",          c_function,          CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# ( % ) > % ;",          c_function_short,    CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# ( % ) ;",              c_call,    CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) ( % ) ;",          c_call,    CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("var % ;",     c_alloc,   CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("val % ;",     c_alloc_const,     CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
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
        cms_add ("GC: % ;",     c_getchar, CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("PUSH: % ;",   c_push,    CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("POP: % ;",    c_pop,     CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("jump % ;",    c_jlabel,  CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("#:",          c_clabel,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! ( % ) { % } { % }", c_ncheck_else,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
 //     cms_add ("! ( % ) > % > % ;",   c_ncheck_else,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! ( % ) { % }",       c_ncheck,       CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! ( % ) > % ;",       c_ncheck_short, CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! # { % } { % }",     c_ncheck_else,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
 //     cms_add ("! # > % > % ;",       c_ncheck_else,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! # { % }",           c_ncheck,       CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("! # > % ;",           c_ncheck_short, CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) { % } { % }",   c_check_else,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
 //     cms_add ("( % ) > % > % ;",     c_check_else,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) { % }",         c_check,        CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) > % ;",         c_check_short,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# { % } { % }",       c_check_else,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
 //     cms_add ("# > % > % ;",         c_check_else,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# { % }",             c_check,        CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# > % ;",             c_check_short,  CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) -> { % }",      c_loop,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# -> { % }",          c_loop,         CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("( % ) -> % ;",        c_loop_short,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("# -> % ;",            c_loop_short,   CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("ret % ;",             c_return,       CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
        cms_add ("inc % ;",             c_include,      CMS_IGNORE_UPPER_LOWER_CASE | CMS_IGNORE_SPACING | CMS_USE_BRACKET_SEARCH_ALGORITHM);
    } ));

   
    // Allocate at register index 0 value 0 -> pointer pointing at 0 have value 0
    fr_register_add (&register_list, REGISTER_ALLOC (VALUE_INT (0)));

    // Search syntax using ´cms_template´ in ´example_text´
    cms_find (code, cms_template);

    if (reset_variables)
        (*variables) = realloc (*variables, sizeof (Variable) * (*pre_variable_count));
    else
        (*pre_variable_count) = variable_count;

    return EXIT_SUCCESS;
}
