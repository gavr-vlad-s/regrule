/*
    File:    main_scaner.cpp
    Created: 23 March 2018 at 15:16 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <cstdio>
#include "../include/main_scaner.h"
#include "../include/knuth_find.h"
#include "../include/main_scaner_keyword_table.h"
#include "../include/belongs.h"
#include "../include/search_char.h"
#include "../include/get_init_state.h"
#include "../include/elem.h"

enum class Category : uint16_t{
    Spaces,          Other,        Percent,
    After_percent,   Id_begin,     Id_body,
    Delimiter_begin, Double_quote, Delimiter_body
};

static const Segment_with_value<char32_t, uint64_t> categories_table[] = {
    {{U'h', U'i'},  56  },  {{U'A', U'Z'},  48  },  {{U's', U't'},  56  },
    {{U',', U'-'},  64  },  {{U'c', U'd'},  56  },  {{U'k', U'n'},  56  },
    {{U'{', U'{'},  64  },  {{U'"', U'"'},  128 },  {{U':', U':'},  64  },
    {{U'a', U'a'},  56  },  {{U'e', U'g'},  48  },  {{U'j', U'j'},  48  },
    {{U'o', U'r'},  48  },  {{U'u', U'z'},  48  },  {{U'}', U'}'},  64  },
    {{   1,   32},  1   },  {{U'%', U'%'},  4   },  {{U'0', U'9'},  32  },
    {{U'>', U'>'},  256 },  {{U'_', U'_'},  48  },  {{U'b', U'b'},  48  }
};

static constexpr size_t num_of_elems_in_categories_table = 21;

static uint64_t get_categories_set(char32_t c)
{
    auto t = knuth_find(categories_table,
                        categories_table + num_of_elems_in_categories_table,
                        c);
    return t.first ? categories_table[t.second].value :
                     (1ULL << static_cast<uint64_t>(Category::Other));
}

Main_scaner::Automaton_proc Main_scaner::procs[] = {
    &Main_scaner::start_proc,     &Main_scaner::unknown_proc,
    &Main_scaner::id_proc,        &Main_scaner::keyword_proc,
    &Main_scaner::delimiter_proc, &Main_scaner::string_proc
};

Main_scaner::Final_proc Main_scaner::finals[] = {
    &Main_scaner::none_final_proc,
    &Main_scaner::unknown_final_proc,
    &Main_scaner::id_final_proc,
    &Main_scaner::keyword_final_proc,
    &Main_scaner::delimiter_final_proc,
    &Main_scaner::string_final_proc
};

static inline uint64_t belongs(Category cat, uint64_t cat_set)
{
    return belongs(static_cast<uint64_t>(cat), cat_set);
}

bool Main_scaner::start_proc(){
    bool t = true;
    state  = -1;
    /* For an automaton that processes a lexeme, the state with the number (-1) is
     * the state in which this automaton is initialized. */
    if(belongs(Category::Spaces, char_categories)){
        loc->current_line += U'\n' == ch;
        return t;
    }
    lexem_begin_line = loc->current_line;
    if(belongs(Category::Percent, char_categories)){
        automaton = A_keyword; token.code = Main_lexem_code::Unknown;
    }else if(belongs(Category::Id_begin, char_categories)){
        buffer.clear();
        automaton  =  A_id;
        buffer     += ch;
        token.code =  Main_lexem_code::Id;
    }else if(belongs(Category::Delimiter_begin, char_categories)){
        automaton = A_delimiter; token.code = Main_lexem_code::Comma;
        (loc->pcurrent_char)--;
    }else if(belongs(Category::Double_quote, char_categories)){
         automaton = A_string; token.code = Main_lexem_code::String;
         buffer.clear(); (loc->pcurrent_char)--;
    }else{
        automaton = A_unknown; token.code = Main_lexem_code::Unknown;
    }
    return t;
}

static const char* keyword_strings[] = {
    "%action",             "%class_members",   "%codes",
    "%codes_type",         "%comments",        "%delimiters",
    "%ident_name",         "%idents",          "%keywords",
    "%multilined",         "%nested",          "%newline_is_lexem",
    "%numbers",            "%scaner_name",     "%single_lined",
    "%strings",            "%token_fields",    "%header_additions",
    "%impl_additions",     "%lexem_info_name"
};

void Main_scaner::correct_keyword(){
    /* This function corrects the lexeme code, which is most likely a keyword, and
     * displays the necessary diagnostics. */
    if(token.code >= Main_lexem_code::M_Kw_action){
        int y = static_cast<int>(token.code) -
                static_cast<int>(Main_lexem_code::M_Kw_action);
        printf("Line %zu expects %s.\n",
               loc->current_line, keyword_strings[y]);
        token.code =
            static_cast<Main_lexem_code>(y +
                                         static_cast<int>(Main_lexem_code::Kw_action));
        en -> increment_number_of_errors();
    }
}

Main_lexem_info Main_scaner::current_lexem(){
    automaton        = A_start; token.code = Main_lexem_code::None;
    lexem_begin      = loc->pcurrent_char;
    lexem_begin_line = loc->current_line;
    bool t           = true;
    while((ch = *(loc->pcurrent_char)++)){
        char_categories = get_categories_set(ch);
        t = (this->*procs[automaton])();
        if(!t){
            /* We get here only if the lexeme has already been read. At the same time,
             * the current automaton reads the character immediately after the end of
             * the lexeme read, based on this symbol, it is decided that the lexeme has
             * been read and the transition to the next character has been made.
             * Therefore, in order to not miss the first character of the next lexeme,
             * we need to decrease the pcurrent_char pointer by one. */
            (loc->pcurrent_char)--;
            if(Main_lexem_code::Id == token.code){
                /* If the current lexeme is an identifier, then this identifier must
                 * be written to the identifier table. */
                token.ident_index = ids -> insert(buffer);
            }else if(Main_lexem_code::String == token.code){
                /* If the current token is a string literal, then it must be written
                 * to the string literal table. */
                token.string_index = strs -> insert(buffer);
            }else if(A_keyword == automaton){
                /* If we finish processing the keyword, then we need to adjust its code,
                 * and, perhaps, output the diagnostics.*/
                correct_keyword();
            }else if(A_delimiter == automaton){
                correct_delim();
            }
            return token;
        }
    }
    /* Here we can be, only if we have already read all the processed text. In this case,
     * the pointer to the current symbol indicates a byte, which is immediately after the
     * zero character, which is a sign of the end of the text. To avoid entering subsequent
     * calls outside the text, we need to go back to the null character. */
    (loc->pcurrent_char)--;
    /* Further, since we are here, the end of the current token (perhaps unexpected) has
     *not yet been processed. It is necessary to perform this processing, and, probably,
     * to display any diagnostics. */
    (this->*finals[automaton])();
    return token;
}

bool Main_scaner::unknown_proc(){
    return belongs(Category::Other, char_categories);
}

bool Main_scaner::id_proc(){
    bool t = belongs(Category::Id_body, char_categories);
    /* The variable t is true if the identifier is not yet
     * fully read, and false otherwise. */
    if(t){
        buffer += ch;
    }
    return t;
}

/* This array consists of pairs of the form (state, character) and is used to
 * initialize the keyword processing automaton. The sense of the element of the
 * array is this: if the current character in the state (-1) coincides with the
 * second component of the element, then work begins with the state that is the
 * first component of the element. Consider, for example, the element {6, U'c '}.
 * If the current character coincides with the second component of this element,
 * then work begins with the state being the first component, i.e. from state 6.
 * The array must be sorted in ascending order of the second component. */
static const State_for_char init_table_for_keywords[] = {
    {0,   U'a'}, {6,   U'c'}, {34,  U'd'}, {44,  U'h'},
    {60,  U'i'}, {84,  U'k'}, {92,  U'l'}, {107, U'm'},
    {117, U'n'}, {143, U's'}, {171, U't'}
};

bool Main_scaner::keyword_proc(){
    bool t = false;
    if(state != -1){
        auto elem  = a_keyword_jump_table[state];
        token.code = elem.code;
        int y = search_char(ch, elem.symbols);
        if(y != THERE_IS_NO_CHAR){
            state = elem.first_state + y; t = true;
        }
        return t;
    }
    if(belongs(Category::After_percent, char_categories)){
        state = get_init_state(ch, init_table_for_keywords,
                               sizeof(init_table_for_keywords)/
                               sizeof(State_for_char));
        token.code = a_keyword_jump_table[state].code;
        t = true;
    }else{
        printf("In line %zu, one of the following symbols is expected: "
               "a, c, d, i, k, m, n, s, t.\n",
               loc->current_line);
        en -> increment_number_of_errors();
    }
    return t;
}

enum {Begin_string = -1, String_body, End_string};
/* These are the state names of the string literals processing automaton. */

bool Main_scaner::string_proc(){
/* This function implements a finite state machine for processing string literals.
 * String literals allowed by Myauka can be described by a regular expression
 *         b(a|bb)*b (1),
 * where b is a double quotation mark ("), and a is any character other than a
 * double quotation mark. In other words, string literals are enclosed in double
 * quotes, and if in a string literal it is necessary to specify such a
 * quotation mark, then it must be doubled. Moreover, from the above regular
 * expression it follows that a string literal can occupy several lines of
 * the source text.
 *
 * Further, if we construct a NFA (a nondeterministic finite state machine) by a
 * this regular expression, then we construct a corresponding DFA (deterministic
 * finite state machine) from the constructed NFA and minimize the resulting DFA,
 * then we obtain a DFA with the following transition table:
 *
 * --------------------------------------------
 * |     state    | a           | b           |
 * --------------------------------------------
 * | Begin_string |             | String_body |
 * --------------------------------------------
 * | String_body  | String_body | End_string  |
 * --------------------------------------------
 * | End_string   |             | String_body |
 * --------------------------------------------
 *
 * In this table, the Begin_string state is initial, and the End_string state
 * is the end state. */
    bool t = true;
    switch(state){
        case Begin_string:
            state = String_body;
            break;
        case String_body:
            if(ch != U'\"'){
                buffer += ch;
            }else{
                state = End_string;
            }
            loc->current_line += U'\n' == ch;
            break;
        case End_string:
            if(U'\"' == ch){
                buffer += ch; state = String_body;
            }else{
                t = false;
            }
            break;
    }
    return t;
}

void Main_scaner::none_final_proc(){
    /* This subroutine will be called if, after reading the input text, it
     * turned out to be in the A_start automaton. Then we do not need to
     * do anything. */
}

void Main_scaner::unknown_final_proc(){
    /* This subroutine will be called if, after reading the input text, it
     * turned out to be in the A_unknown automaton. Then we do not need to
     * do anything. */
}

void Main_scaner::id_final_proc(){
    /* This function will be called if, after reading the input stream, it
     *appeared in the identifier processing automaton, the A_id. Then the
     * identifier must be written to the identifier table. */
    token.ident_index = ids -> insert(buffer);
}

void Main_scaner::keyword_final_proc(){
    token.code = a_keyword_jump_table[state].code;
    correct_keyword();
}

void Main_scaner::string_final_proc(){
    /* This function will be called if, after reading the input stream, it appeared in
     * the string literals processing automaton. If in this case we are not in the
     * final state of this automaton, then we must display the diagnosis. */
    token.string_index = strs -> insert(buffer);
    if(state != End_string){
        printf("Unexpected end of string literal at line %zu.\n",
               loc->current_line);
        en -> increment_number_of_errors();
    }
}

static const Elem<Main_lexem_code> a_delim_jump_table[] = {
    {const_cast<char32_t*>(U""),  Main_lexem_code::Comma,            0},   // 0
    {const_cast<char32_t*>(U""),  Main_lexem_code::Colon,            0},   // 1
    {const_cast<char32_t*>(U""),  Main_lexem_code::Opened_fig_brack, 0},   // 2
    {const_cast<char32_t*>(U""),  Main_lexem_code::Closed_fig_brack, 0},   // 3
    {const_cast<char32_t*>(U">"), Main_lexem_code::M_arrow,          5},   // 4
    {const_cast<char32_t*>(U""),  Main_lexem_code::Arrow,            0},   // 5
};

static const State_for_char init_table_for_delims[] = {
    {0,   U','}, {4,   U'-'}, {1,  U':'},
    {2,   U'{'}, {3,   U'}'}
};

bool Main_scaner::delimiter_proc()
{
    bool t = false;
    if(state != -1){
        auto elem  = a_delim_jump_table[state];
        token.code = elem.code;
        int y      = search_char(ch, elem.symbols);
        if(y != THERE_IS_NO_CHAR){
            state = elem.first_state + y; t = true;
        }
        return t;
    }
    state = get_init_state(ch, init_table_for_delims,
                           sizeof(init_table_for_delims)/
                           sizeof(State_for_char));
    token.code = a_keyword_jump_table[state].code;
    t = true;
    return t;
}

void Main_scaner::correct_delim()
{
    /* This function corrects the lexeme code, which is most likely a delimiter, and
     * displays the necessary diagnostics. */
    if(token.code == Main_lexem_code::M_arrow){
        printf("Unexpected character at line %zu. Expected character >.\n",
               loc->current_line);
        token.code = Main_lexem_code::Arrow;
        en -> increment_number_of_errors();
    }
}

void Main_scaner::delimiter_final_proc(){
    token.code = a_delim_jump_table[state].code;
    correct_delim();
}